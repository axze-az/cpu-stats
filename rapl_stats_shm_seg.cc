//
//  Copyright (C) 2020-2022  Axel Zeuner
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
#include "rapl_stats.h"
#include "tools.h"
#include <sstream>
#include <iomanip>
#include <cmath>

constexpr const double rapl_stats::shm_seg::power_step;
constexpr const double rapl_stats::shm_seg::inv_power_step;
constexpr const double rapl_stats::shm_seg::max_power;

std::string
rapl_stats::shm_seg::name(std::uint32_t pkg)
{
    std::ostringstream s;
    s << "/cpu_stats_p_pkg_" << std::setw(3) << std::setfill('0') << pkg;
    return s.str();
}

rapl_stats::shm_seg::shm_seg(std::uint32_t pkg)
    : _pkg(pkg),
      _uj_lo(0), _uj_hi(0),
      _entries{0}
{
}

rapl_stats::shm_seg::~shm_seg()
{
    std::string fn=name(_pkg);
    tools::shm::unlink(fn);
}

rapl_stats::shm_seg*
rapl_stats::shm_seg::create(std::uint32_t cpu)
{
    std::string fn=name(cpu);
    void* addr=tools::shm::create(fn, sizeof(shm_seg), 0644);
    shm_seg* ret=new (addr) shm_seg(cpu);
    return ret;
}

void
rapl_stats::shm_seg::close(shm_seg* p)
{
    p->~shm_seg();
    tools::shm::unmap(p, sizeof(shm_seg));
}

const rapl_stats::shm_seg*
rapl_stats::shm_seg::open(std::uint32_t cpu)
{
    std::string fn=name(cpu);
    void* addr=tools::shm::open_ro(fn, sizeof(shm_seg));
    const shm_seg* ret=reinterpret_cast<const shm_seg*>(addr);
    return ret;
}

void
rapl_stats::shm_seg::close(const shm_seg* p)
{
    void* ap=const_cast<shm_seg*>(p);
    tools::shm::unmap(ap, sizeof(shm_seg));
}

std::size_t
rapl_stats::shm_seg::power_to_idx(double p_in_w)
{
    double p0=std::floor(p_in_w*inv_power_step);
    auto i=static_cast<std::size_t>(p0);
    i=std::min(i, std::size_t(POWER_ENTRIES)-1);
    return i;
}

double
rapl_stats::shm_seg::idx_to_power(std::size_t idx)
{
    return (1+idx)*power_step;
}
