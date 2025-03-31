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
#if !defined (__CPUFREQ_STATS_H__)
#define __CPUFREQ_STATS_H__ 1

#include <tools.h>
#include <cstdint>
#include <vector>
#include <string>
#include <iosfwd>

namespace cpufreq_stats {


    struct cpu {
        static
        std::string path(std::uint32_t cpu);
        static
        bool exists(std::uint32_t cpu);
        static
        bool online(std::uint32_t cpu);
        static
        double min_freq(std::uint32_t cpu);
        static
        double max_freq(std::uint32_t cpu);
        static
        double cur_freq(std::uint32_t cpu);
    };

    // shared memory segment between server and client, one per
    // logical core
    class shm_seg {
        shm_seg(std::uint32_t cpu);
        ~shm_seg();
        static
        std::string name(std::uint32_t cpu_num);
        // frequency step of 200 MHz/XXX khz
        static
        constexpr const double freq_step=200000;
        static
        constexpr const double inv_freq_step=1.0/freq_step;
        // maximum frequency = 7GHz for now
        static
        constexpr const double max_freq=7000000;
    public:
        enum {
            FREQ_ENTRIES=uint32_t(max_freq/freq_step)+1
        };
    private:
        // cpu number
        std::uint32_t _cpu;
        // min frequency
        double _min_f_khz;
        // max frequency
        double _max_f_khz;
        // last measured frequency
        double _last_f_khz;
        // array with ticks/freq range, _entries[0] 0*freq_step,
        // _entries[1] 1*freq_step, ..
        // (2^32)-1)/(3600*24*365.25) ~ 136.09 years are possible
        // if only one _entries[C] is used.
        std::uint32_t _entries[FREQ_ENTRIES];
    public:
        static
        shm_seg*
        create(std::uint32_t cpu_num);

        static
        void
        close(shm_seg* p);

        static
        const shm_seg*
        open(std::uint32_t cpu_num);

        static
        void
        close(const shm_seg* p);

        static
        double
        round_f(double f);

        static
        std::size_t
        freq_to_idx(double f);

        static
        double
        idx_to_freq(std::uint32_t f);

        const std::uint32_t& cpu() const;
        const double& min_f_khz() const;
        const double& max_f_khz() const;
        shm_seg& last_f_khz(const double& f);
        const double& last_f_khz() const;
        std::uint32_t* begin();
        std::uint32_t* end();
        const std::uint32_t* begin() const;
        const std::uint32_t* end() const;
    };

    class data {
        std::vector<const shm_seg*> _v;
        bool _create;

        static
        void
        to_stream(std::ostream& s, const shm_seg* p, bool short_output);
    public:
        data(bool create);
        ~data();
        data(const data&) = delete;
        data&
        operator=(const data& r) = delete;
        // update _v
        void
        update(std::uint32_t weight);
        // dump the data
        void
        to_stream(std::ostream& s, bool short_output=false);
    };

}

inline
const std::uint32_t&
cpufreq_stats::shm_seg::cpu()
    const
{
    return _cpu;
}

inline
const double&
cpufreq_stats::shm_seg::min_f_khz()
    const
{
    return _min_f_khz;
}

inline
const double&
cpufreq_stats::shm_seg::max_f_khz()
    const
{
    return _max_f_khz;
}

inline
cpufreq_stats::shm_seg&
cpufreq_stats::shm_seg::last_f_khz(const double& v)
{
    _last_f_khz = v;
    return *this;
}

inline
const double&
cpufreq_stats::shm_seg::last_f_khz()
    const
{
    return _last_f_khz;
}

inline
std::uint32_t*
cpufreq_stats::shm_seg::begin()
{
    return _entries;
}

inline
std::uint32_t*
cpufreq_stats::shm_seg::end()
{
    return _entries+FREQ_ENTRIES;
}

inline
const std::uint32_t*
cpufreq_stats::shm_seg::begin()
    const
{
    return _entries;
}

inline
const std::uint32_t*
cpufreq_stats::shm_seg::end()
    const
{
    return _entries+FREQ_ENTRIES;
}

// Local variables:
// mode: c++
// end:
#endif
