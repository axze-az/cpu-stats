#include "cpufreq_stats.h"
#include "power_stats.h"
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

#define RUN_DIR "/run"

int write_pidfile()
{
    char fname[PATH_MAX];
    mode_t m;
    int lockfd;
    struct flock lck;
    snprintf(fname, sizeof(fname), RUN_DIR "/cpufreq-stats-daemon.pid");
    m = umask(0);
    lockfd=open(fname, O_RDWR | O_CREAT,
                S_IWUSR | S_IRUSR | S_IRGRP | S_IROTH);
    if ( lockfd > -1 ) {
        lck.l_type = F_WRLCK;
        lck.l_whence = SEEK_SET;
        lck.l_start = 0;
        lck.l_len = 0;
        if ((fcntl(lockfd, F_SETLK, &lck)<0) ||
            (fcntl (lockfd, F_SETFD, FD_CLOEXEC)< 0)) {
            int t=-errno;
            close(lockfd);
            lockfd=t;
        } else {
            char pidbuf[64];
            ssize_t s=snprintf(pidbuf,sizeof(pidbuf),
                                "%ld\n", (long)getpid());
            if ((s >= (ssize_t)sizeof(pidbuf)) ||
                (write(lockfd,pidbuf,s) != s)) {
                int t=-errno;
                close(lockfd);
                lockfd=t;
            }
        }
    } else {
        lockfd = -errno;
    }
    umask(m);
    return lockfd;
}

int daemon_main(bool foreground, std::uint32_t timeout)
{
    try {
        openlog("cpufreq-stats-daemon",
                LOG_PID | (foreground ? LOG_PERROR :0), LOG_DAEMON);
        if (foreground == false) {
            if (daemon(0,0) < 0) {
                syslog(LOG_ERR, "Cannot daemonize");
                exit(3);
            }
        }
        /* int lock_fd=-1; */
        if (getuid()==0) {
            write_pidfile();
            int nr=nice(-20);
            if (nr != -20) {
                syslog(LOG_WARNING, "Could not set nice(-20)");
            }
#if 0
            struct passwd *pwe;
            struct group *ge;
            if ((pwe = getpwnam("daemon")) == NULL) {
                syslog(LOG_ERR, "Cannot get uid for daemon");
                exit(3);
            }
            uid_t daemon_uid = pwe->pw_uid;
            if ((ge = getgrnam("daemon")) == NULL)
                syslog(LOG_ERR, "Cannot get gid for daemon");
            gid_t daemon_gid = ge->gr_gid;
            if (setegid(daemon_gid)!=0) {
                syslog(LOG_ERR, "Cannot set gid for daemon");
                exit(3);
            }
            if (seteuid(daemon_uid)!=0) {
                syslog(LOG_ERR, "Cannot set uid for daemon");
                exit(3);
            }
#endif
        }
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
        // every 2 seconds
        iv.it_interval.tv_sec=timeout;
        // and the next one in timeout second
        iv.it_value.tv_nsec=0;
        iv.it_value.tv_sec=timeout;
        timer_settime(timerid, 0, &iv, 0);
        power_stats::data p_dta(true);
        cpufreq_stats::data f_dta(true);

        sigset_t s;
        sigfillset(&s);
        siginfo_t si;
        syslog(LOG_INFO,
               "version 0.2 startup complete using a timeout of %d seconds.",
               timeout);
        bool done=false;
        while (!done) {
            int wr=sigwaitinfo(&s, &si);
            if (wr==sev.sigev_signo && si.si_value.sival_ptr==&timerid) {
                int tmr_or=timer_getoverrun(timerid);
                if (tmr_or != -1) {
                    for (int i=0; i<=tmr_or; ++i)
                        f_dta.update();
                    p_dta.update((tmr_or+1)*timeout);
                } else {
                    // log an error here later
                    f_dta.update();
                }
            } else {
                switch (wr) {
                case SIGALRM:
                    f_dta.update();
                    break;
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
            std::ostringstream os;
            p_dta.to_stream(os);
            std::istringstream is(os.str());
            std::string l;
            while (std::getline(is, l).good()) {
                syslog(LOG_INFO, "%s", l.c_str());
            }
        }
        {
            std::ostringstream os;
            f_dta.to_stream(os);
            std::istringstream is(os.str());
            std::string l;
            while (std::getline(is, l).good()) {
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
              << "-t X  sample every X seconds, 0<X<=60, default 2\n"
              << "-h    print this information and exit\n";
    std::exit(3);
}

int main(int argc, char** argv)
{
    char c;
    std::int32_t timeout=2;
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
    return daemon_main(foreground, timeout);
}
