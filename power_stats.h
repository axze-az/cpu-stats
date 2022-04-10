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
        // powerstep of 5 W's
        static
        constexpr const double power_step=5;
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
        uint32_t _pkg;
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
        update(std::uint32_t tmo_sec);
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
