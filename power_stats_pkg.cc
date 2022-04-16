#include "power_stats.h"
#include <sstream>

std::string
power_stats::pkg::path(std::uint32_t no)
{
    std::ostringstream s;
    s << "/sys/devices/virtual/powercap/intel-rapl/intel-rapl:" << no << '/';
    return s.str();
}

bool
power_stats::pkg::exists(std::uint32_t no)
{
    std::string p=path(no);
    return tools::file::exists(p);
}

std::uint32_t
power_stats::pkg::enabled(std::uint32_t no)
{
    std::string p=path(no) + "enabled";
    return tools::sys_fs::read<std::int32_t>::from(p);
}

std::uint64_t
power_stats::pkg::energy_uj(std::uint32_t no)
{
    std::string p=path(no) + "energy_uj";
    return tools::sys_fs::read<std::uint64_t>::from(p);
}

std::uint64_t
power_stats::pkg::max_energy_range_uj(std::uint32_t no)
{
    std::string p=path(no) + "max_energy_range_uj";
    return tools::sys_fs::read<std::uint64_t>::from(p);
}

