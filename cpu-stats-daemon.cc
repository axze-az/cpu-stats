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
#include "cpu-stats.h"
#include "cpufreq_stats.h"
#include "rapl_stats.h"
#include "amdgpu_stats.h"
#include <unistd.h>
#include <sys/time.h>
#include <time.h>
#include <pwd.h>
#include <grp.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <limits.h>

#include <syslog.h>
#include <cstring>
#include <sstream>
#include <iostream>

constexpr const std::uint32_t default_timeout_seconds=3;
#define RUN_DIR "/run"

tools::file_handle write_pidfile()
{
    const char* fname=RUN_DIR "/cpu-stats-daemon.pid";
    tools::scoped_zero_umask zero_umask;
    tools::file_handle lockfd=open(fname, O_RDWR | O_CREAT,
        S_IWUSR | S_IRUSR | S_IRGRP | S_IROTH);
    if (lockfd() > -1) {
        struct flock lck;
        lck.l_type = F_WRLCK;
        lck.l_whence = SEEK_SET;
        lck.l_start = 0;
        lck.l_len = 0;
        if ((fcntl(lockfd(), F_SETLK, &lck)<0) ||
            (fcntl(lockfd(), F_SETFD, FD_CLOEXEC)< 0)) {
            lockfd=tools::file_handle(-errno);
        } else {
            std::ostringstream pids;
            pids << getpid();
            std::string pidbuf(pids.str());
            ssize_t s=pidbuf.length();
            if (write(lockfd(), pidbuf.c_str(), s) != s) {
                lockfd=tools::file_handle(-errno);
            }
        }
    } else {
        lockfd=tools::file_handle(-errno);
    }
    return lockfd;
}

int daemon_main(bool foreground, std::uint32_t timeout)
{
    try {
        openlog("cpu-stats-daemon",
                LOG_PID | (foreground ? LOG_PERROR :0), LOG_DAEMON);
        if (foreground == false) {
            if (daemon(0,0) < 0) {
                syslog(LOG_ERR, "Cannot daemonize");
                std::exit(3);
            }
        }
        /* int lock_fd=-1; */
        tools::file_handle pid_fd=write_pidfile();
        if (pid_fd()<0) {
            syslog(LOG_ERR, "could not write pid file, errno %i\n", -pid_fd());
            std::exit(3);
        }
        int nr=nice(-20);
        if (nr != -20) {
            syslog(LOG_WARNING, "Could not set nice(-20)");
        }
#if 0
        struct passwd *pwe;
        struct group *ge;
        if ((pwe = getpwnam("daemon")) == NULL) {
            syslog(LOG_ERR, "Cannot get uid for daemon");
            std::exit(3);
        }
        uid_t daemon_uid = pwe->pw_uid;
        if ((ge = getgrnam("daemon")) == NULL)
            syslog(LOG_ERR, "Cannot get gid for daemon");
        gid_t daemon_gid = ge->gr_gid;
        if (setegid(daemon_gid)!=0) {
            syslog(LOG_ERR, "Cannot set gid for daemon");
            std::exit(3);
        }
        if (seteuid(daemon_uid)!=0) {
            syslog(LOG_ERR, "Cannot set uid for daemon");
            std::exit(3);
        }
#endif
        // block all signals
        tools::block_signals blk;
        timer_t timerid;
        struct sigevent sev;
        sev.sigev_notify = SIGEV_SIGNAL;
        sev.sigev_signo = SIGRTMIN;
        sev.sigev_value.sival_ptr = &timerid;
        timer_create(CLOCK_REALTIME, &sev, &timerid);
        struct itimerspec iv;
        std::memset(&iv, 0, sizeof(iv));
        // every timeout seconds
        iv.it_interval.tv_sec=timeout;
        // and the next one in timeout second
        iv.it_value.tv_nsec=0;
        iv.it_value.tv_sec=timeout;
        timer_settime(timerid, 0, &iv, 0);
        rapl_stats::data r_dta(true);
        amdgpu_stats::data g_dta(true);
        cpufreq_stats::data f_dta(true);

        sigset_t s;
        sigfillset(&s);
        siginfo_t si;
        syslog(LOG_INFO,
               "version %s startup complete using a timeout of %u seconds.",
               cpu_stats::version,
               timeout);
        bool done=false;
        while (!done) {
            int wr=sigwaitinfo(&s, &si);
            if ((wr==sev.sigev_signo) && (si.si_value.sival_ptr==&timerid)) {
                int tmr_or=timer_getoverrun(timerid);
                if (tmr_or != -1) {
                    std::uint32_t weight=tmr_or+1;
                    f_dta.update(weight);
                    r_dta.update(weight*timeout, weight);
                    g_dta.update(weight*timeout, weight);
                    if (weight > 1) {
                        syslog(LOG_WARNING,
                               "timer_getoverrun() returned %d", tmr_or);
                    }
                } else {
                    syslog(LOG_ERR,
                           "timer_getoverrun() errno=%d", errno);
                }
            } else {
                switch (wr) {
                case SIGTERM:
                case SIGINT:
                case SIGQUIT:
                    done=true;
                    syslog(LOG_INFO, "shutting down");
                    break;
                default:
                    break;
                }
            }
        }
        {
            std::stringstream s;
            r_dta.to_stream(s, false);
            std::string l;
            while (std::getline(s, l).good()) {
                syslog(LOG_INFO, "%s", l.c_str());
            }
        }
        {
            std::stringstream s;
            g_dta.to_stream(s, false);
            std::string l;
            while (std::getline(s, l).good()) {
                syslog(LOG_INFO, "%s", l.c_str());
            }
        }
        {
            std::stringstream s;
            f_dta.to_stream(s, false);
            std::string l;
            while (std::getline(s, l).good()) {
                syslog(LOG_INFO, "%s", l.c_str());
            }
        }
    }
    catch (const std::runtime_error& e) {
        std::cerr << e.what() << '\n';
        std::cerr << "Is another instance of the daemon running?\n";
        return 3;
    }
    return 0;
}

void
usage(const char* argv)
{
    std::cerr << argv << " [-f] [-t X] [-h]\n"
              << "-f    stay in foreground\n"
              << "-t X  sample every X seconds, 0<X<=60, default "
              << default_timeout_seconds<< "\n"
              << "-h    print this information and exit\n";
    std::exit(3);
}

int main(int argc, char** argv)
{
    char c;
    std::int32_t timeout=default_timeout_seconds;
    bool foreground=false;
    while ((c=getopt(argc, argv, "hft:")) != -1) {
        switch (c) {
        case 'f':
            foreground=true;
            break;
        case 't':
            timeout=std::atoi(optarg);
            break;
        case 'h':
        default:
            usage(argv[0]);
            break;
        }
    }
    if (timeout < 1 || timeout > 60)
        usage(argv[0]);
    if (geteuid()!=0) {
        std::cerr << "this program requires root rights for access "
                     "to rapl data"
                  << std::endl;
        std::exit(3);
    }
    return daemon_main(foreground, timeout);
}
