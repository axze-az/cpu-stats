#include "cpufreq_stats.h"
#include <sstream>
#include <fstream>

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

