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
#include "tools.h"
#include <sstream>
#include <iomanip>
#include <cmath>

constexpr const double amdgpu_stats::shm_seg::power_step;
constexpr const double amdgpu_stats::shm_seg::inv_power_step;
constexpr const double amdgpu_stats::shm_seg::max_power;

std::string
amdgpu_stats::shm_seg::name(std::uint32_t hwmon)
{
    std::ostringstream s;
    s << "/cpu_stats_p_amdgpu_" << std::setw(3) << std::setfill('0') << hwmon;
    return s.str();
}

amdgpu_stats::shm_seg::shm_seg(std::uint32_t id)
    : _id(id),
      _power(0.0),
      _elapsed_s(0),
      _entries{0}
{
}

amdgpu_stats::shm_seg::~shm_seg()
{
    std::string fn=name(_id);
    tools::shm::unlink(fn);
}

amdgpu_stats::shm_seg*
amdgpu_stats::shm_seg::create(std::uint32_t hwmon)
{
    std::string fn=name(hwmon);
    void* addr=tools::shm::create(fn, sizeof(shm_seg), 0644);
    shm_seg* ret=new (addr) shm_seg(hwmon);
    return ret;
}

void
amdgpu_stats::shm_seg::close(shm_seg* p)
{
    p->~shm_seg();
    tools::shm::unmap(p, sizeof(shm_seg));
}

const amdgpu_stats::shm_seg*
amdgpu_stats::shm_seg::open(std::uint32_t hwmon)
{
    std::string fn=name(hwmon);
    void* addr=tools::shm::open_ro(fn, sizeof(shm_seg));
    const shm_seg* ret=reinterpret_cast<const shm_seg*>(addr);
    return ret;
}

void
amdgpu_stats::shm_seg::close(const shm_seg* p)
{
    void* ap=const_cast<shm_seg*>(p);
    tools::shm::unmap(ap, sizeof(shm_seg));
}

std::size_t
amdgpu_stats::shm_seg::power_to_idx(double p_in_w)
{
    double p0=std::floor(p_in_w*inv_power_step);
    auto i=static_cast<std::size_t>(p0);
    i=std::min(i, std::size_t(POWER_ENTRIES)-1);
    return i;
}

double
amdgpu_stats::shm_seg::idx_to_power(std::size_t idx)
{
    return (1+idx)*power_step;
}
