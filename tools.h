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
#if !defined (__TOOLS_H__)
#define __TOOLS_H__ 1

#include <signal.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstdint>
#include <cstddef>
#include <string>
#include <fstream>

namespace tools {

    // block all signals in the current scope
    class block_signals {
        sigset_t _old;
    public:
        block_signals(const block_signals& r)=delete;
        block_signals& operator=(const block_signals& r)=delete;
        block_signals(block_signals&& r)=delete;
        block_signals& operator=(block_signals&& r)=delete;
        block_signals();
        ~block_signals();
    };

    // set umask to zero in the current scope
    class scoped_zero_umask {
        mode_t _om;
    public:
        scoped_zero_umask(const scoped_zero_umask& r)=delete;
        scoped_zero_umask& operator=(const scoped_zero_umask& r)=delete;
        scoped_zero_umask(scoped_zero_umask&& r)=delete;
        scoped_zero_umask& operator=(scoped_zero_umask&& r)=delete;
        scoped_zero_umask();
        ~scoped_zero_umask();
    };

    // close fd at the end of the current scope
    class file_handle {
        int _fd;
        void _close();
    public:
        file_handle(const file_handle& r)=delete;
        file_handle& operator=(const file_handle& r)=delete;
        file_handle(int fd);
        file_handle(file_handle&& r);
        file_handle& operator=(file_handle&& r);
        ~file_handle();
        const int& operator()() const;
    };

    class iarraybuf : public std::streambuf {
    public:
        iarraybuf(const char* base, size_t size);
    };

    class iarraystream : private iarraybuf, public std::istream {
    public:
        iarraystream(const char* base, size_t size);
        iarraystream(const std::string_view& s);
    };

    // named shared memory
    namespace shm {
        // create a new shared memory posix file fname with size s and
        // and mode m and map it into memory
        void*
        create(const std::string& fname, std::size_t s, mode_t m=0644);
        // open shared memory posix file fname
        // and map s bytes of it read only into memory
        void*
        open_ro(const std::string& fname, std::size_t s);
        // unmap p
        void
        unmap(void* p, std::size_t s);
        // delete file name
        void
        unlink(const std::string& fname);
    };

    namespace file {
        bool
        exists(const std::string& fn);
    }

    namespace sys_fs {
        // helper struct to read<_T>::from files
        template <typename _T>
        struct read {
            static
            _T
            from(const std::string& fn);
        };

        template <>
        struct read<std::string> {
            static
            std::string
            from(const std::string& fn);
        };
    }
}

inline
tools::block_signals::block_signals()
{
    sigset_t s;
    sigfillset(&s);
    pthread_sigmask(SIG_SETMASK, &s, &_old);
}

inline
tools::block_signals::~block_signals()
{
    pthread_sigmask(SIG_SETMASK, &_old, nullptr);
}

inline
tools::scoped_zero_umask::scoped_zero_umask()
    : _om(umask(0))
{
}

inline
tools::scoped_zero_umask::~scoped_zero_umask()
{
    umask(_om);
}

inline
void
tools::file_handle::_close()
{
    if (_fd >= 0)
        close(_fd);
}

inline
tools::file_handle::file_handle(int fd)
    : _fd(fd)
{
}

inline
tools::file_handle::file_handle(file_handle&& r)
    : _fd(r._fd)
{
    r._fd=-1;
}

inline
tools::file_handle&
tools::file_handle::operator=(file_handle&& r)
{
    _close();
    _fd=r._fd;
    r._fd=-1;
    return *this;
}

inline
tools::file_handle::~file_handle()
{
    _close();
}

inline
const int&
tools::file_handle::operator()() const
{
    return _fd;
}

template <typename _T>
_T
tools::sys_fs::read<_T>::from(const std::string& fn)
{
#if 1
    std::string fc=read<std::string>::from(fn);
    iarraystream s(fc);
#else
    std::ifstream s(fn.c_str());
#endif
    _T r(0);
    s >> r;
    return r;
}


#endif
