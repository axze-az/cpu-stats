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
#include <sstream>

std::string
rapl_stats::pkg::path(std::uint32_t no)
{
    std::ostringstream s;
    s << "/sys/devices/virtual/powercap/intel-rapl/intel-rapl:" << no << '/';
    return s.str();
}

bool
rapl_stats::pkg::exists(std::uint32_t no)
{
    std::string p=path(no);
    return tools::file::exists(p);
}

std::uint32_t
rapl_stats::pkg::enabled(std::uint32_t no)
{
    std::string p=path(no) + "enabled";
    return tools::sys_fs::read<std::int32_t>::from(p);
}

std::uint64_t
rapl_stats::pkg::energy_uj(std::uint32_t no)
{
    std::string p=path(no) + "energy_uj";
    return tools::sys_fs::read<std::uint64_t>::from(p);
}

std::uint64_t
rapl_stats::pkg::max_energy_range_uj(std::uint32_t no)
{
    std::string p=path(no) + "max_energy_range_uj";
    return tools::sys_fs::read<std::uint64_t>::from(p);
}

