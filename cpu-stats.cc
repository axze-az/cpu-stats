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
#include "cpu-stats.h"
#include "cpufreq_stats.h"
#include "rapl_stats.h"
#include "amdgpu_stats.h"
#include <iostream>
#include <string_view>


namespace {
    void
    usage(const std::string_view& argv0)
    {
	std::cerr << argv0
		  << " [-v|--version] [-s|--short] [-l|--long] \n"
		  << "-s|--short     requests short output\n"
		  << "-l|--long      requests long output\n"
		  << "-p|--power     requests power output only\n"
		  << "-f|--frequency requests frequency output only\n"
		  << "-v|--version   displays version informantion\n";
	std::exit(3);
    }
}

int main(int argc, char** argv)
{
    bool short_output=true;
    bool power_only=false;
    bool frequency_only=false;
    for (int argi = 1; argi < argc; ++argi) {
        std::string_view ag(argv[argi]);
        if (ag=="-v" || ag=="--version") {
            std::cout << argv[0] << " version " << cpu_stats::version << '\n';
            return 0;
        } else if (ag=="-s" || ag=="--short") {
            short_output=true;
        } else if (ag=="-l" || ag=="--long") {
            short_output=false;
        } else if (ag=="-p" || ag=="--power") {
            power_only=true;
        } else if (ag=="-f" || ag=="--frequency") {
	    frequency_only=true;
        } else {
	    usage(argv[0]);
        }
    }
    bool output_power=(power_only==false && frequency_only==false) ||
	(power_only==true);
    bool output_frequency=(power_only==false && frequency_only==false) ||
	(frequency_only==true);
    if (output_power) {
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
    }
    if (output_frequency) {
	try {
	    cpufreq_stats::data dta(false);
	    dta.to_stream(std::cout, short_output);
	}
	catch (const std::runtime_error& e) {
	    std::cerr << e.what() << '\n';
	    std::cerr << "Is the daemon running?\n";
	}
    }
    return 0;
}
