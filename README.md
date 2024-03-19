[![License:MIT](https://img.shields.io/badge/License-MIT-blue?style=plastic)](LICENSE)
[![C++ CI build](https://github.com/oskarirauta/cpu_cpp/actions/workflows/build.yml/badge.svg)](https://github.com/oskarirauta/cpu_cpp/actions/workflows/build.yml)
### cpu_cpp

CPU and process monitoring library

## <sub>Depends</sub>

 - [throws_cpp](https://github.com/oskarirauta/throws_cpp)
 - [common_cpp](https://github.com/oskarirauta/common_cpp)
 - [logger_cpp](https://github.com/oskarirauta/logger_cpp)

## <sub>Importing</sub>

 - clone throws_cpp as a submodule to throws
 - clone common_cpp as a submodule to common
 - clone logger_cpp as a submodule to logger
 - clone cpu_cpp as a submodule to cpu
 - include throws_cpp's, common_cpp's and logger_cpp's and cpu_cpp's Makefile.incs in your Makefile
 - link with THROWS_OBJS, COMMON_OBJS, LOGGER_OBJS and CPU_OBJS

Paths are modifiable, check Makefiles. For example code, clone this repository with
--recursive-submodules enabled.

## <sub>Example</sub>

Sample code for monitoring cpu usage and
single processe's resource usage is provided


