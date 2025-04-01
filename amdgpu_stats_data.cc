//
//  Copyright (C) 2020-2025  Axel Zeuner
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//
#include "amdgpu_stats.h"
#include <iostream>
#include <iomanip>
#include <cmath>
#include <algorithm>
#include <numeric>
#include <syslog.h>

amdgpu_stats::data::data(bool create)
    : _v(),
      _create(create)
{
    try {
        for (size_t i=0; hwmon::exists(i); ++i) {
            if (!hwmon::is_amdgpu(i))
                continue;
            const shm_seg* p=nullptr;
            if (_create) {
                p=shm_seg::create(i);
            } else {
                p=shm_seg::open(i);
            }
            _v.push_back(p);
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

amdgpu_stats::data::~data()
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
amdgpu_stats::data::
update(std::uint32_t tmo_sec, std::uint32_t weight)
{
    if (_create == false)
        return;
    for (std::size_t i=0; i<_v.size(); ++i) {
        shm_seg* p=const_cast<shm_seg*>(_v[i]);
        std::uint64_t p_in_uw=hwmon::ppt(p->id());
        double p_in_w = double(p_in_uw)*1e-6;
        // std::cout << " p in w: " << p_in_w << '\n';
        size_t idx=shm_seg::power_to_idx(p_in_w);
        if ((idx>=shm_seg::POWER_ENTRIES-1) ||
            (p_in_w > shm_seg::max_power)) {
            syslog(LOG_INFO,
                   "amdgpu_stats: reading from amdgpu: "
                   "%f",
                   p_in_w);
        }
        // std::cout << " idx: " << idx << std::endl;
        std::uint64_t s=p->elapsed_s() + tmo_sec;
        std::uint32_t* pi=p->begin() + idx;
        (*pi)+=weight;
        p->power(p_in_w);
        p->elapsed_s(s);
    }
}

void
amdgpu_stats::data::
to_stream(std::ostream& s, const shm_seg* p, bool short_output)
{
    std::uint32_t vt[shm_seg::POWER_ENTRIES];
    std::uint32_t id= p->id();
    std::copy(p->begin(), p->end(), std::begin(vt));

    // determine entries != 0
    std::size_t cnt=0;
    std::size_t vidx[shm_seg::POWER_ENTRIES];
    double vpct[shm_seg::POWER_ENTRIES];
    std::size_t idx_max=0;
    std::uint32_t max_ti=0;
    double sum_ti=0.0;
    double p_in_w=p->power();
    std::uint64_t elapsed_s=p->elapsed_s();
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
    s << "amdgpu-hwmon " << id
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
            s << std::setw(5) << std::setprecision(1) << pi << ' '
              << std::setw(7) << std::setprecision(2) << pcti << ' '
              << std::setw(7) << std::setprecision(2) << spcti;
            sum += pcti;
            avg += pi*pcti;
        }
        s << '\n';
    }
    avg = avg*1.0e-2 - (shm_seg::power_step*.5);
    double ws=double(elapsed_s)*avg;
    double kwh=ws/(1000*3600);
    ws = rint(ws);
    kwh= rint(kwh*1e3)*1e-3;
    s << "average power: ~" << avg << " W, power over last interval: "
      << p_in_w << " W\n"
      << "used energy:   ~"
      << std::scientific << std::setprecision(15) << ws << " Ws, ~"
      << std::setprecision(15) << kwh << " kWh"
      << '\n';
    if (std::fabs(sum-100) > 0.005) {
        s << "invalid sum " << sum << std::endl;
    }
}

void
amdgpu_stats::data::to_stream(std::ostream& s, bool short_output)
{
    for (std::size_t i=0; i<_v.size(); ++i) {
        to_stream(s, _v[i], short_output);
    }
}
