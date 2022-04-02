#include "cpufreq_stats.h"
#include "power_stats.h"
#include <iostream>

int main()
{
    try {
        power_stats::data dta(false);
        dta.to_stream(std::cout);
    }
    catch (const std::runtime_error& e) {
        std::cerr << e.what() << '\n';
        std::cerr << "Is the daemon running?\n";
    }
    try {
        cpufreq_stats::data dta(false);
        dta.to_stream(std::cout);
    }
    catch (const std::runtime_error& e) {
        std::cerr << e.what() << '\n';
        std::cerr << "Is the daemon running?\n";
    }
    return 0;
}
