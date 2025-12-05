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
#if !defined (__AMDGPU_STATS_H__)
#define __AMDGPU_STATS_H__ 1

#include <tools.h>
#include <cstdint>
#include <vector>

namespace amdgpu_stats {

    struct hwmon {
        static
        std::string path(std::uint32_t no);

        static
        bool
        exists(std::uint32_t no);

        static
        bool
        is_amdgpu(std::uint32_t no);

        static
        bool
        has_ppt(std::uint32_t no);

        static
        std::uint64_t
        ppt(std::uint32_t no);

    };

    class shm_seg {
        shm_seg(std::uint32_t id);
        ~shm_seg();
        static
        std::string name(std::uint32_t id);
    public:
        // powerstep of 2.5 W's
        static
        constexpr const double power_step=2.5;
        static
        constexpr const double inv_power_step=1.0/power_step;
        // until we figure out
        static
        constexpr const double max_power=350;
        enum {
            POWER_ENTRIES=uint32_t(max_power/power_step)+1
        };
    private:
        // hwmon device id
        std::uint32_t _id;
        // power read last time
        double _power;
        // (bad) estimation of time elapsed
        std::uint64_t _elapsed_s;
        // array with ticks/power_range
        std::uint32_t _entries[POWER_ENTRIES];
    public:
        static
        shm_seg*
        create(std::uint32_t pkg);

        static
        void
        close(shm_seg* p);

        static
        const shm_seg*
        open(std::uint32_t pkg);

        static
        void
        close(const shm_seg* p);

        static
        std::size_t
        power_to_idx(double p_in_w);

        static
        double
        idx_to_power(std::size_t p);

        const std::uint32_t& id() const;
        shm_seg& power(const double& pwr);
        const double& power() const;
        shm_seg& elapsed_s(const std::uint64_t& v);
        const std::uint64_t& elapsed_s() const;
        std::uint32_t* begin();
        std::uint32_t* end();
        const std::uint32_t* begin() const;
        const std::uint32_t* end() const;
    };

    struct data {
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
        update(std::uint32_t tmo_sec, std::uint32_t weight);
        // dump the data
        void
        to_stream(std::ostream& s, bool short_output=false);
    };
}

inline
const std::uint32_t&
amdgpu_stats::shm_seg::id()
    const
{
    return _id;
}

inline
amdgpu_stats::shm_seg&
amdgpu_stats::shm_seg::power(const double& v)
{
    _power=v;
    return *this;
}

inline
const double&
amdgpu_stats::shm_seg::power()
    const
{
    return _power;
}

inline
amdgpu_stats::shm_seg&
amdgpu_stats::shm_seg::elapsed_s(const std::uint64_t& v)
{
    _elapsed_s=v;
    return *this;
}

inline
const std::uint64_t&
amdgpu_stats::shm_seg::elapsed_s()
    const
{
    return _elapsed_s;
}

inline
std::uint32_t*
amdgpu_stats::shm_seg::begin()
{
    return _entries;
}

inline
std::uint32_t*
amdgpu_stats::shm_seg::end()
{
    return _entries+POWER_ENTRIES;
}

inline
const std::uint32_t*
amdgpu_stats::shm_seg::begin()
    const
{
    return _entries;
}

inline
const std::uint32_t*
amdgpu_stats::shm_seg::end()
    const
{
    return _entries+POWER_ENTRIES;
}


#endif
