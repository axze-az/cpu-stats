#include "cpufreq_stats.h"
#include "tools.h"
#include <sstream>
#include <iomanip>
#include <cmath>

const double cpufreq_stats::shm_seg::freq_step;
const double cpufreq_stats::shm_seg::inv_freq_step;
const double cpufreq_stats::shm_seg::max_freq;

std::string
cpufreq_stats::shm_seg::name(std::uint32_t cpu)
{
    std::ostringstream s;
    s << "/cpu_stats_f_cpu_" << std::setw(3) << std::setfill('0') << cpu;
    return s.str();
}

cpufreq_stats::shm_seg::shm_seg(std::uint32_t cpu)
    : _cpu(cpu),
      _min_f_khz(cpu::min_freq(cpu)),
      _max_f_khz(cpu::max_freq(cpu)),
      _entries{0}
{
}

cpufreq_stats::shm_seg::~shm_seg()
{
    std::string fn=name(_cpu);
    tools::shm::unlink(fn);
}

cpufreq_stats::shm_seg*
cpufreq_stats::shm_seg::create(std::uint32_t cpu)
{
    std::string fn=name(cpu);
    void* addr=tools::shm::create(fn, sizeof(shm_seg), 0644);
    shm_seg* ret=new (addr) shm_seg(cpu);
    return ret;
}

void
cpufreq_stats::shm_seg::close(shm_seg* p)
{
    p->~shm_seg();
    tools::shm::unmap(p, sizeof(shm_seg));
}

const cpufreq_stats::shm_seg*
cpufreq_stats::shm_seg::open(std::uint32_t cpu)
{
    std::string fn=name(cpu);
    void* addr=tools::shm::open_ro(fn, sizeof(shm_seg));
    const shm_seg* ret=reinterpret_cast<const shm_seg*>(addr);
    return ret;
}

void
cpufreq_stats::shm_seg::close(const shm_seg* p)
{
    void* ap=const_cast<shm_seg*>(p);
    tools::shm::unmap(ap, sizeof(shm_seg));
}

double
cpufreq_stats::shm_seg::round_f(double f)
{
    return std::rint(f*inv_freq_step)*freq_step;
}

std::size_t
cpufreq_stats::shm_seg::freq_to_idx(double f)
{
    double f0=std::rint(f*inv_freq_step);
    auto i=static_cast<std::size_t>(f0);
    i = std::min(i, std::size_t(FREQ_ENTRIES)-1);
    return i;
}

double
cpufreq_stats::shm_seg::idx_to_freq(std::uint32_t i)
{
    return i * freq_step;
}
