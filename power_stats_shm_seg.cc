#include "power_stats.h"
#include "tools.h"
#include <sstream>
#include <iomanip>
#include <cmath>

constexpr const double power_stats::shm_seg::power_step;
constexpr const double power_stats::shm_seg::inv_power_step;
constexpr const double power_stats::shm_seg::max_power;

std::string
power_stats::shm_seg::name(std::uint32_t pkg)
{
    std::ostringstream s;
    s << "/cpu_stats_p_pkg_" << std::setw(3) << std::setfill('0') << pkg;
    return s.str();
}

power_stats::shm_seg::shm_seg(std::uint32_t pkg)
    : _pkg(pkg),
      _entries{0}
{
}

power_stats::shm_seg::~shm_seg()
{
    std::string fn=name(_pkg);
    tools::shm::unlink(fn);
}

power_stats::shm_seg*
power_stats::shm_seg::create(std::uint32_t cpu)
{
    std::string fn=name(cpu);
    void* addr=tools::shm::create(fn, sizeof(shm_seg), 0644);
    shm_seg* ret=new (addr) shm_seg(cpu);
    return ret;
}

void
power_stats::shm_seg::close(shm_seg* p)
{
    p->~shm_seg();
    tools::shm::unmap(p, sizeof(shm_seg));
}

const power_stats::shm_seg*
power_stats::shm_seg::open(std::uint32_t cpu)
{
    std::string fn=name(cpu);
    void* addr=tools::shm::open_ro(fn, sizeof(shm_seg));
    const shm_seg* ret=reinterpret_cast<const shm_seg*>(addr);
    return ret;
}

void
power_stats::shm_seg::close(const shm_seg* p)
{
    void* ap=const_cast<shm_seg*>(p);
    tools::shm::unmap(ap, sizeof(shm_seg));
}

std::size_t
power_stats::shm_seg::power_to_idx(double p_in_w)
{
    double p0=std::floor(p_in_w*inv_power_step);
    auto i=static_cast<std::size_t>(p0);
    i=std::min(i, std::size_t(POWER_ENTRIES)-1);
    return i;
}

double
power_stats::shm_seg::idx_to_power(std::size_t idx)
{
    return (1+idx)*power_step;
}
