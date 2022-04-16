#include "cpufreq_stats.h"
#include "power_stats.h"
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
        power_stats::data dta(false);
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
