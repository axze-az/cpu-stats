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
#include "cpu-stats.h"
#include "cpufreq_stats.h"
#include "rapl_stats.h"
#include "amdgpu_stats.h"
#include <iostream>
#include <string_view>

int main(int argc, char** argv)
{
    bool short_output=false;
    if (argc > 1) {
        std::string_view ag1(argv[1]);
        if (ag1=="-s" || ag1=="--short") {
            short_output=true;
        } else {
            std::cerr << argv[0] << " [-s|--short]\n"
                      << "-s|--short  requests short output\n";
            return 3;
        }
    }
    try {
        rapl_stats::data dta(false);
        dta.to_stream(std::cout, short_output);
    }
    catch (const std::runtime_error& e) {
        std::cerr << e.what() << '\n';
        std::cerr << "Is the daemon running?\n";
    }
    try {
        amdgpu_stats::data dta(false);
        dta.to_stream(std::cout, short_output);
    }
    catch (const std::runtime_error& e) {
        std::cerr << e.what() << '\n';
        std::cerr << "Is the daemon running?\n";
    }
    try {
        cpufreq_stats::data dta(false);
        dta.to_stream(std::cout, short_output);
    }
    catch (const std::runtime_error& e) {
        std::cerr << e.what() << '\n';
        std::cerr << "Is the daemon running?\n";
    }
    return 0;
}
