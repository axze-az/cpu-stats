# makefile for cpufreq-statsd and cpufreq-stats

IROOT=${DESTDIR}
PREFIX=/home/cpufreq
BIN_DIR=${PREFIX}/usr/bin
SBIN_DIR=${PREFIX}/usr/sbin
INIT_DIR=${PREFIX}/etc/init.d

all: cpu-stats-daemon cpu-stats

STRIP=-s
CXX=g++
CXXFLAGS=-pipe -O2 -fomit-frame-pointer -Wall -I.
#CXXFLAGS+= -ffunction-sections -fdata-sections
LD=$(CXX)
LIBS=-L. -lcpustats -lrt -lpthread
LDFLAGS=$(CXXFLAGS) $(STRIP) #-static-libstdc++
OBJS= \
cpufreq_stats_cpu.o \
cpufreq_stats_shm_seg.o \
cpufreq_stats_data.o \
rapl_stats_pkg.o \
rapl_stats_shm_seg.o \
rapl_stats_data.o \
amdgpu_stats_hwmon.o \
amdgpu_stats_shm_seg.o \
amdgpu_stats_data.o \
tools.o

cpu-stats-daemon: cpu-stats-daemon.o libcpustats.a
	$(LD) $(LDFLAGS) -o $@ $< $(LIBS)

cpu-stats: cpu-stats.o libcpustats.a
	$(LD) $(LDFLAGS) -o $@ $< $(LIBS)

libcpustats.a: $(OBJS)
	$(AR) r $@ $?

clean:
	-$(RM) cpu-stats-daemon cpu-stats libcpustats.a *.o *.s

distclean: clean
	-$(RM) *~

install: all
	mkdir -p ${IROOT}/${BIN_DIR}
	install -m 0755 -g root -o root cpu-stats ${IROOT}/${BIN_DIR}
	mkdir -p ${IROOT}/${SBIN_DIR}
	install -m 0755 -g root -o root cpu-stats-daemon ${IROOT}/${SBIN_DIR}

HEADERS=cpufreq_stats.h rapl_stats.h amdgpu_stats.h tools.h 
cpu-stats-daemon.o: cpu-stats-daemon.cc $(HEADERS)
cpu-stats.o: cpu-stats.cc $(HEADERS)
cpufreq_stats_cpu.o: cpufreq_stats_cpu.cc cpufreq_stats.h tools.h
cpufreq_stats_shm_seg.o: cpufreq_stats_shm_seg.cc cpufreq_stats.h tools.h
cpufreq_stats_data.o: cpufreq_stats_shm_seg.cc cpufreq_stats.h tools.h
rapl_stats_pkg.o: rapl_stats_pkg.cc rapl_stats.h tools.h
rapl_stats_shm_seg.o: rapl_stats_shm_seg.cc rapl_stats.h tools.h
rapl_stats_data.o: rapl_stats_data.cc rapl_stats.h tools.h
amdgpu_stats_hwmon.o: amdgpu_stats_hwmon.cc amdgpu_stats.h tools.h
amdgpu_stats_shm_seg.o: amdgpu_stats_shm_seg.cc amdgpu_stats.h tools.h
amdgpu_stats_data.o: amdgpu_stats_data.cc amdgpu_stats.h tools.h
tools.o: tools.cc tools.h


