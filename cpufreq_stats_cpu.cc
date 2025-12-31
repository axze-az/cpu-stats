//
//  Copyright (C) 2020-2026  Axel Zeuner
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
#include "cpufreq_stats.h"
#include <sstream>

std::string
cpufreq_stats::cpu::path(std::uint32_t cpu)
{
    std::ostringstream s;
    s << "/sys/devices/system/cpu/cpu" << cpu << '/';
    return s.str();
}


bool
cpufreq_stats::cpu::exists(std::uint32_t cpu)
{
    std::string p=path(cpu);
    return tools::file::exists(p);
}

bool
cpufreq_stats::cpu::online(std::uint32_t cpu)
{
    if (cpu==0)
        return true;
    std::string p=path(cpu)+"online";
    int r=tools::sys_fs::read<std::int32_t>::from(p);
    return r!=0;
}

double
cpufreq_stats::cpu::max_freq(std::uint32_t cpu)
{
    std::string p=path(cpu)+"cpufreq/cpuinfo_max_freq";
    return tools::sys_fs::read<double>::from(p);
}

double
cpufreq_stats::cpu::min_freq(std::uint32_t cpu)
{
    std::string p=path(cpu)+"cpufreq/cpuinfo_min_freq";
    return tools::sys_fs::read<double>::from(p);
}

double
cpufreq_stats::cpu::cur_freq(std::uint32_t cpu)
{
    if (!online(cpu))
        return 0.0;
    std::string p=path(cpu)+"cpufreq/scaling_cur_freq";
    return tools::sys_fs::read<double>::from(p);
}

