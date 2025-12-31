//
//  Copyright (C) 2020-2026  Axel Zeuner
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
#include "tools.h"
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdexcept>

tools::iarraybuf::
iarraybuf(const char* base, size_t size)
{
    char* p=const_cast<char*>(base);
    this->setg(p, p, p + size);
}

tools::iarraystream::
iarraystream(const char* base, size_t size)
    : iarraybuf(base, size),
      std::istream(static_cast<std::streambuf*>(this))
{
}

tools::iarraystream::
iarraystream(const std::string_view& s)
    : iarraybuf(s.data(), s.length()),
      std::istream(static_cast<std::streambuf*>(this))
{
}

void*
tools::shm::
create(const std::string& fname, std::size_t s, mode_t m)
{
    tools::scoped_zero_umask um;
    tools::file_handle fd(
        shm_open(fname.c_str(), O_RDWR|O_CREAT|O_EXCL, m));
    if (fd()==-1) {
        std::string msg="could not create shm " + fname;
        throw std::runtime_error(msg);
    }
    if (ftruncate(fd(), s)!=0) {
        unlink(fname);
        std::string msg="could not resize shm " + fname;
        throw std::runtime_error(msg);
    }
    void* addr=mmap(nullptr,
                    s,
                    PROT_READ|PROT_WRITE,
                    MAP_SHARED,
                    fd(),
                    0);
    if (addr==MAP_FAILED) {
        unlink(fname);
        std::string msg="could not map shm " + fname;
        throw std::runtime_error(msg);
    }
    return addr;
}

void*
tools::shm::open_ro(const std::string& fname, std::size_t s)
{
    tools::file_handle fd(
        shm_open(fname.c_str(), O_RDONLY, 0));
    if (fd()==-1) {
        std::string msg="could not open shm " + fname;
        throw std::runtime_error(msg);
    }
    void* addr=mmap(nullptr,
                    s,
                    PROT_READ,
                    MAP_SHARED,
                    fd(),
                    0);
    if (addr==MAP_FAILED) {
        std::string msg="could not map shm " + fname;
        throw std::runtime_error(msg);
    }
    return addr;
}

void
tools::shm::
unmap(void* p, std::size_t s)
{
    munmap(p, s);
}

void
tools::shm::
unlink(const std::string& fname)
{
    shm_unlink(fname.c_str());
}

bool
tools::file::exists(const std::string& fn)
{
    struct stat st;
    bool r=false;
    if (stat(fn.c_str(), &st) == 0)
        r=true;
    return r;
}

std::string
tools::sys_fs::read<std::string>::from(const std::string& fn)
{
    std::string r;
    file_handle fd=open(fn.c_str(), O_RDONLY);
    if (fd() <0) {
        return r;
    }
#if 0
    struct stat st;
    if (fstat(fd(), &st)<0) {
        return r;
    }
    std::string::size_type fs=st.st_size;
#else
    std::string::size_type fs=4096;
#endif
    r.resize(fs, 0);
    ssize_t rs=0;
    if ((rs=::read(fd(), r.data(), ssize_t(fs))) != ssize_t(fs)) {
        r.resize(std::max(rs, ssize_t(0)));
    }
    return r;
}

