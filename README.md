# cpu-stats

cpu-stats is a simple daemon monitoring the cpus of a linux system

## Description

cpu-stats consists of a daemon monitoring the power consumption of the
cpus and the frequencies of the cores and a client program accessing
these collected data.

The daemon requires access to the rapl information of the system to
monitor the power consumption of the packages. The minimum power
sampling rate is 1 time per second (1Hz) in order to avoid leaking
information out of the kernel, 

## Getting Started

### Dependencies
- the program works only under linux
- it has never been compiled with a compiler other than gcc

### Build and test

- Execute `fakeroot debian/rules binary` to produce a debian package 
- otherwise you may edit the Makefile in the root directory to change
  paths and compile it

## License

This project is licensed under the GPL v2.0 License.
