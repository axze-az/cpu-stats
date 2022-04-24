#include "cpufreq_stats.h"
#include <iostream>
#include <iomanip>
#include <cmath>
#include <algorithm>
#include <numeric>

cpufreq_stats::data::data(bool create)
    : _v(), _create(create)
{
    try {
        for (size_t i=0; cpu::exists(i); ++i) {
            if (_create) {
                shm_seg* p=shm_seg::create(i);
                _v.push_back(p);
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

cpufreq_stats::data::~data()
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
cpufreq_stats::data::update(std::uint32_t weight)
{
    if (_create == false)
        return;
    for (std::size_t i=0; i<_v.size(); ++i) {
        shm_seg* p=const_cast<shm_seg*>(_v[i]);
        double cur_f=cpu::cur_freq(p->cpu());
        size_t idx=shm_seg::freq_to_idx(cur_f);
        std::uint32_t* pi=p->begin() + idx;
        (*pi)+=weight;
    }
}

void
cpufreq_stats::data::
to_stream(std::ostream& s, const shm_seg* p, bool short_output)
{
    std::uint32_t cpu;
    double min_f, max_f;
    std::uint32_t vt[shm_seg::FREQ_ENTRIES];
    cpu= p->cpu();
    min_f=p->min_f_khz();
    max_f=p->max_f_khz();
    std::copy(p->begin(), p->end(), std::begin(vt));

    // determine entries != 0
    std::size_t cnt=0;
    std::size_t vidx[shm_seg::FREQ_ENTRIES];
    double vpct[shm_seg::FREQ_ENTRIES];
    std::size_t idx_max=0;
    std::uint32_t max_ti=0;
    double sum_ti=0.0;
    for (std::size_t i=0; i<shm_seg::FREQ_ENTRIES; ++i) {
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
    double vspct[shm_seg::FREQ_ENTRIES];
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
    s << "cpu " << cpu
      << ", f_min=" << min_f/1000
      << ", f_max=" << max_f/1000
      << ", samples=" << std::scientific << std::setprecision(22) << sum_ti
      << std::fixed << '\n';
    if (!short_output) {
        for (std::uint32_t i=0; i<cols; ++i) {
            if (i)
                s << " | ";
            s << "f/MHz       %   sum % ";
        }
        s << '\n';
    }
    std::uint32_t lines=(cnt+cols-1)/cols;
    double sum=0.0, avg=0.0;
    for (std::uint32_t j=0; j<lines; ++j) {
        for (std::uint32_t i=0; i<cols; ++i) {
            std::size_t k=j+lines*i;
            if (k >= cnt)
                continue;
            std::size_t idx = vidx[k];
            double fi=shm_seg::idx_to_freq(idx);
            fi /= 1000;
            double pcti=vpct[k];
            double spcti=vspct[k];
            if (!short_output) {
                if (i)
                    s << "  | ";
                s << std::setw(5) << std::setprecision(0) << fi << ' '
                << std::setw(7) << std::setprecision(2) << pcti << ' '
                << std::setw(7) << std::setprecision(2) << spcti;
            }
            sum +=pcti;
            avg +=pcti*fi;
        }
        if (!short_output)
            s << '\n';
    }
    avg *= 1e-2;
    s << "average frequency: ~" << std::setprecision(0) << avg << " MHz\n";
    if (std::fabs(sum-100) > 0.005) {
        s << "invalid sum " << sum << std::endl;
    }
}

void
cpufreq_stats::data::to_stream(std::ostream& s, bool short_output)
{
    for (std::size_t i=0; i<_v.size(); ++i) {
        to_stream(s, _v[i], short_output);
    }
}
