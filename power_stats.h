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
#if !defined (__POWER_STATS_H__)
#define __POWER_STATS_H__ 1

#include <tools.h>
#include <vector>

namespace power_stats {

    struct pkg {
        static
        std::string path(std::uint32_t no);

        static
        bool
        exists(std::uint32_t no);

        static
        std::uint32_t
        enabled(std::uint32_t no);

        static
        std::uint64_t
        energy_uj(std::uint32_t no);

        static
        std::uint64_t
        max_energy_range_uj(std::uint32_t no);
    };

    class shm_seg {
        shm_seg(std::uint32_t pkg);
        ~shm_seg();
        static
        std::string name(std::uint32_t pkg);
    public:
        // powerstep of 2.5 W's
        static
        constexpr const double power_step=2.5;
        static
        constexpr const double inv_power_step=1.0/power_step;
        // until we figure out
        static
        constexpr const double max_power=250;
        enum {
            POWER_ENTRIES=uint32_t(max_power/power_step)+1
        };
    private:
        // package number
        std::uint32_t _pkg;
        // micro joules since start
        std::uint64_t _uj_lo;
        std::uint64_t _uj_hi;
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

        const std::uint32_t& pkg() const;
        const std::uint64_t& uj_lo() const;
        shm_seg& uj_lo(const std::uint64_t& uj);
        const std::uint64_t& uj_hi() const;
        shm_seg& uj_hi(const std::uint64_t& uj);
        std::uint32_t* begin();
        std::uint32_t* end();
        const std::uint32_t* begin() const;
        const std::uint32_t* end() const;
    };

    struct data {
        std::vector<const shm_seg*> _v;
        struct priv_data {
            std::uint64_t _energy_uj;
            std::uint64_t _max_energy_range_uj;
        };
        std::vector<priv_data> _vp;
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
power_stats::shm_seg::pkg()
    const
{
    return _pkg;
}

inline
const std::uint64_t&
power_stats::shm_seg::uj_lo()
    const
{
    return _uj_lo;
}

inline
power_stats::shm_seg&
power_stats::shm_seg::uj_lo(const std::uint64_t& v)
{
    _uj_lo = v;
    return *this;
}

inline
const std::uint64_t&
power_stats::shm_seg::uj_hi()
    const
{
    return _uj_hi;
}

inline
power_stats::shm_seg&
power_stats::shm_seg::uj_hi(const std::uint64_t& v)
{
    _uj_hi = v;
    return *this;
}

inline
std::uint32_t*
power_stats::shm_seg::begin()
{
    return _entries;
}

inline
std::uint32_t*
power_stats::shm_seg::end()
{
    return _entries+POWER_ENTRIES;
}

inline
const std::uint32_t*
power_stats::shm_seg::begin()
    const
{
    return _entries;
}

inline
const std::uint32_t*
power_stats::shm_seg::end()
    const
{
    return _entries+POWER_ENTRIES;
}


#endif
