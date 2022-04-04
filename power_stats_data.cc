#include "power_stats.h"
#include <iostream>
#include <iomanip>
#include <cmath>
#include <algorithm>
#include <numeric>
#include <syslog.h>

power_stats::data::data(bool create)
    : _v(),
      _v_energy_uj(),
      _create(create)
{
    try {
        for (size_t i=0; pkg::exists(i); ++i) {
            if (_create) {
                shm_seg* p=shm_seg::create(i);
                _v.push_back(p);
                std::uint64_t e=pkg::energy_uj(i);
                _v_energy_uj.push_back(e);
            } else {
                const shm_seg* p=shm_seg::open(i);
                _v.push_back(p);
            }
        }
    }
    catch (const std::runtime_error& e) {
        if (_create) {
            for (size_t i=0; i<_v.size(); ++i) {
                shm_seg* p=const_cast<shm_seg*>(_v[i]);
                shm_seg::close(p);
            }
        }
        throw;
    }
}

power_stats::data::~data()
{
    if (_create==true) {
        for (std::size_t i=0; i<_v.size(); ++i) {
            shm_seg* p=const_cast<shm_seg*>(_v[i]);
            shm_seg::close(p);
        }
    } else {
        for (std::size_t i=0; i<_v.size(); ++i) {
            shm_seg::close(_v[i]);
        }
    }
}

void
power_stats::data::update(std::uint32_t tmo_sec)
{
    if (_create == false)
        return;
    for (std::size_t i=0; i<_v.size(); ++i) {
        shm_seg* p=const_cast<shm_seg*>(_v[i]);
        std::uint64_t e_now=pkg::energy_uj(p->pkg());
        // std::cout << "e_now: " << e_now;
        std::uint64_t e_last=_v_energy_uj[i];
        // std::cout << " e_last: " << e_last;
        if (e_now <= e_last) {
            syslog(LOG_INFO,
                   "power_stats: reading from rapl: "
                   "now: %lu last: %lu with timeout %u",
                   e_now, e_last, tmo_sec);
        }
        _v_energy_uj[i]=e_now;
        // conversion factor between ujoule and joule and
        // division by time to obtain power in watt
        double factor= 1.0e-6/double(tmo_sec);
        double p_in_w = (e_now - e_last)*factor;
        // std::cout << " p in w: " << p_in_w;
        size_t idx=shm_seg::power_to_idx(p_in_w);
        if ((idx>=shm_seg::POWER_ENTRIES-1) ||
            (p_in_w > shm_seg::max_power)) {
            syslog(LOG_INFO,
                   "power_stats: reading from rapl: "
                   "now: %lu last: %lu with timeout %u and p: %f",
                   e_now, e_last, tmo_sec, p_in_w);
        }
        // std::cout << " idx: " << idx << std::endl;
        std::uint32_t* pi=p->begin() + idx;
        ++(*pi);
    }
}

void
power_stats::data::
to_stream(std::ostream& s, const shm_seg* p)
{
    std::uint32_t vt[shm_seg::POWER_ENTRIES];
    std::uint32_t pkg= p->pkg();
    std::copy(p->begin(), p->end(), std::begin(vt));

    // determine entries != 0
    std::size_t cnt=0;
    std::size_t vidx[shm_seg::POWER_ENTRIES];
    double vpct[shm_seg::POWER_ENTRIES];
    std::size_t idx_max=0;
    std::uint32_t max_ti=0;
    double sum_ti=0.0;
    for (std::size_t i=0; i<shm_seg::POWER_ENTRIES; ++i) {
        std::uint32_t ti=vt[i];
        if (vt[i]==0)
            continue;
        vidx[cnt]=i;
        double dti=double(ti);
        sum_ti += dti;
        vpct[cnt]=dti;
        if (ti > max_ti) {
            idx_max = cnt;
            max_ti = ti;
        }
        ++cnt;
    }
    // produce percents from the ticks in vpct
    double sum_pct=0.0;
    for (std::size_t i=0; i<cnt; ++i) {
        double pcti=(vpct[i]*1e2)/sum_ti;
        pcti=std::rint(1e2*pcti)*1e-2;
        if (i != idx_max)
            sum_pct += pcti;
        vpct[i]=pcti;
    }
    vpct[idx_max] = 100.0 - sum_pct;
    double vspct[shm_seg::POWER_ENTRIES];
    std::partial_sum(std::begin(vpct), std::begin(vpct)+cnt,
                     std::begin(vspct));
    std::reverse(std::begin(vidx), std::begin(vidx)+cnt);
    std::reverse(std::begin(vpct), std::begin(vpct)+cnt);
    std::reverse(std::begin(vspct), std::begin(vspct)+cnt);

    const std::uint32_t cols=3;
    for (std::uint32_t i=0; i<cols; ++i)
        s << "========================";
    s << '\n';
    s << std::fixed << std::setprecision(0);
    s << "package " << pkg
      << ", samples=" << std::scientific << std::setprecision(22) << sum_ti
      << std::fixed << '\n';
    for (std::uint32_t i=0; i<cols; ++i) {
        if (i)
            s << " | ";
        s << "Pwr/W       %   sum % ";
    }
    s << '\n';
    std::uint32_t lines=(cnt+cols-1)/cols;
    double sum=0.0, avg=0.0;
    for (std::uint32_t j=0; j<lines; ++j) {
        for (std::uint32_t i=0; i<cols; ++i) {
            std::size_t k=j+lines*i;
            if (k >= cnt)
                continue;
            std::size_t idx = vidx[k];
            double pi=shm_seg::idx_to_power(idx);
            double pcti=vpct[k];
            double spcti=vspct[k];
            if (i)
                s << "  | ";
            s << std::setw(5) << std::setprecision(0) << pi << ' '
              << std::setw(7) << std::setprecision(2) << pcti << ' '
              << std::setw(7) << std::setprecision(2) << spcti;
            sum += pcti;
            avg += pi*pcti;
        }
        s << '\n';
    }
    avg = avg*1.0e-2 - (shm_seg::power_step*.5);
    s << "average power: ~" << avg << " W\n";
    if (std::fabs(sum-100) > 0.005) {
        s << "invalid sum " << sum << std::endl;
    }
}

void
power_stats::data::to_stream(std::ostream& s)
{
    for (std::size_t i=0; i<_v.size(); ++i) {
        to_stream(s, _v[i]);
    }
}
