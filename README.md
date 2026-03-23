# cpu-stats

cpu-stats is a simple daemon monitoring the cpus of a linux system

## Description

cpu-stats consists of a daemon monitoring the power consumption of the
cpus and of amd gpus and the frequencies of the cores and a client
program accessing these collected data.

The daemon requires access to the rapl information of the system to
monitor the power consumption of the packages. The minimum power
sampling rate is 1 time per second (1Hz) in order to avoid leaking
information out of the kernel,

It is possible to pass through the collected data into lxc containers.

## Getting Started

### Dependencies
- the program works only under linux
- it has never been compiled with a compiler other than gcc

### Build and test

- Execute `fakeroot debian/rules binary` to produce a debian package 
- otherwise you may edit the Makefile in the root directory to change
  paths and compile it

### Data pass through into lxc containers

Add the following lines to the configuration file of the container:

```
# lxc.mount entries for cpu-stats
lxc.mount.entry = none dev/shm tmpfs nodev,nosuid,noexec,mode=1777,create=dir 0 0
lxc.mount.entry=/dev/shm/cpu_stats_p_pkg_000 dev/shm/cpu_stats_p_pkg_000 none bind,ro,optional,create=file
lxc.mount.entry=/dev/shm/cpu_stats_p_amdgpu_000 dev/shm/cpu_stats_p_amdgpu_000 none bind,ro,optional,create=file
lxc.mount.entry=/dev/shm/cpu_stats_f_cpu_000 dev/shm/cpu_stats_f_cpu_000 none bind,ro,optional,create=file
lxc.mount.entry=/dev/shm/cpu_stats_f_cpu_001 dev/shm/cpu_stats_f_cpu_001 none bind,ro,optional,create=file
lxc.mount.entry=/dev/shm/cpu_stats_f_cpu_002 dev/shm/cpu_stats_f_cpu_002 none bind,ro,optional,create=file
lxc.mount.entry=/dev/shm/cpu_stats_f_cpu_003 dev/shm/cpu_stats_f_cpu_003 none bind,ro,optional,create=file
lxc.mount.entry=/dev/shm/cpu_stats_f_cpu_004 dev/shm/cpu_stats_f_cpu_004 none bind,ro,optional,create=file
lxc.mount.entry=/dev/shm/cpu_stats_f_cpu_005 dev/shm/cpu_stats_f_cpu_005 none bind,ro,optional,create=file
lxc.mount.entry=/dev/shm/cpu_stats_f_cpu_006 dev/shm/cpu_stats_f_cpu_006 none bind,ro,optional,create=file
lxc.mount.entry=/dev/shm/cpu_stats_f_cpu_007 dev/shm/cpu_stats_f_cpu_007 none bind,ro,optional,create=file
lxc.mount.entry=/dev/shm/cpu_stats_f_cpu_008 dev/shm/cpu_stats_f_cpu_008 none bind,ro,optional,create=file
lxc.mount.entry=/dev/shm/cpu_stats_f_cpu_009 dev/shm/cpu_stats_f_cpu_009 none bind,ro,optional,create=file
lxc.mount.entry=/dev/shm/cpu_stats_f_cpu_010 dev/shm/cpu_stats_f_cpu_010 none bind,ro,optional,create=file
lxc.mount.entry=/dev/shm/cpu_stats_f_cpu_011 dev/shm/cpu_stats_f_cpu_011 none bind,ro,optional,create=file
lxc.mount.entry=/dev/shm/cpu_stats_f_cpu_012 dev/shm/cpu_stats_f_cpu_012 none bind,ro,optional,create=file
lxc.mount.entry=/dev/shm/cpu_stats_f_cpu_013 dev/shm/cpu_stats_f_cpu_013 none bind,ro,optional,create=file
lxc.mount.entry=/dev/shm/cpu_stats_f_cpu_014 dev/shm/cpu_stats_f_cpu_014 none bind,ro,optional,create=file
lxc.mount.entry=/dev/shm/cpu_stats_f_cpu_015 dev/shm/cpu_stats_f_cpu_015 none bind,ro,optional,create=file
lxc.mount.entry=/dev/shm/cpu_stats_f_cpu_016 dev/shm/cpu_stats_f_cpu_016 none bind,ro,optional,create=file
lxc.mount.entry=/dev/shm/cpu_stats_f_cpu_017 dev/shm/cpu_stats_f_cpu_017 none bind,ro,optional,create=file
lxc.mount.entry=/dev/shm/cpu_stats_f_cpu_018 dev/shm/cpu_stats_f_cpu_018 none bind,ro,optional,create=file
lxc.mount.entry=/dev/shm/cpu_stats_f_cpu_019 dev/shm/cpu_stats_f_cpu_019 none bind,ro,optional,create=file
lxc.mount.entry=/dev/shm/cpu_stats_f_cpu_020 dev/shm/cpu_stats_f_cpu_020 none bind,ro,optional,create=file
lxc.mount.entry=/dev/shm/cpu_stats_f_cpu_021 dev/shm/cpu_stats_f_cpu_021 none bind,ro,optional,create=file
lxc.mount.entry=/dev/shm/cpu_stats_f_cpu_022 dev/shm/cpu_stats_f_cpu_022 none bind,ro,optional,create=file
lxc.mount.entry=/dev/shm/cpu_stats_f_cpu_023 dev/shm/cpu_stats_f_cpu_023 none bind,ro,optional,create=file
lxc.mount.entry=/dev/shm/cpu_stats_f_cpu_024 dev/shm/cpu_stats_f_cpu_024 none bind,ro,optional,create=file
lxc.mount.entry=/dev/shm/cpu_stats_f_cpu_025 dev/shm/cpu_stats_f_cpu_025 none bind,ro,optional,create=file
lxc.mount.entry=/dev/shm/cpu_stats_f_cpu_026 dev/shm/cpu_stats_f_cpu_026 none bind,ro,optional,create=file
lxc.mount.entry=/dev/shm/cpu_stats_f_cpu_027 dev/shm/cpu_stats_f_cpu_027 none bind,ro,optional,create=file
lxc.mount.entry=/dev/shm/cpu_stats_f_cpu_028 dev/shm/cpu_stats_f_cpu_028 none bind,ro,optional,create=file
lxc.mount.entry=/dev/shm/cpu_stats_f_cpu_029 dev/shm/cpu_stats_f_cpu_029 none bind,ro,optional,create=file
lxc.mount.entry=/dev/shm/cpu_stats_f_cpu_030 dev/shm/cpu_stats_f_cpu_030 none bind,ro,optional,create=file
lxc.mount.entry=/dev/shm/cpu_stats_f_cpu_031 dev/shm/cpu_stats_f_cpu_031 none bind,ro,optional,create=file
```

The number of /dev/shm/cpu_stats_f_cpu_XXX lines should match the
number of visible cores in the container.

Install the cpu-stats package install the container.

## License

This project is licensed under the GPL v2.0 License.
