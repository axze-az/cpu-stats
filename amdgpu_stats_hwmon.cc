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
#include <sstream>
#include <iostream>

std::string
amdgpu_stats::hwmon::path(std::uint32_t no)
{
    std::ostringstream s;
    s << "/sys/class/hwmon/hwmon" << no << '/';
    return s.str();
}

bool
amdgpu_stats::hwmon::exists(std::uint32_t no)
{
    std::string p=path(no);
    return tools::file::exists(p);
}

std::uint32_t
amdgpu_stats::hwmon::is_amdgpu(std::uint32_t no)
{
    std::string p=path(no) + "name";
    std::string n=tools::sys_fs::read<std::string>::from(p);
    return n=="amdgpu\n";
}

std::uint64_t
amdgpu_stats::hwmon::ppt(std::uint32_t no)
{
    std::string p=path(no) + "power1_input";
    return tools::sys_fs::read<std::uint64_t>::from(p);
}
