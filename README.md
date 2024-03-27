[![License:MIT](https://img.shields.io/badge/License-MIT-blue?style=plastic)](LICENSE)
[![C++ CI build](../../actions/workflows/build.yml/badge.svg)](../../actions/workflows/build.yml)
### cpu_cpp

CPU and process monitoring library

## <sub>Depends</sub>

 - [common_cpp](https://github.com/oskarirauta/common_cpp)
 - [throws_cpp](https://github.com/oskarirauta/throws_cpp)
 - [scanner_cpp](https://github.com/oskarirauta/scanner_cpp)
 - [logger_cpp](https://github.com/oskarirauta/logger_cpp)

## <sub>Importing</sub>

 - clone common_cpp as a submodule to common
 - clone throws_cpp as a submodule to throws
 - clone scanner_cpp as a submodule to scanner
 - clone logger_cpp as a submodule to logger
 - clone cpu_cpp as a submodule to cpu
 - include common_cpp's, throws_cpp's, scanner_cpp's  and logger_cpp's and cpu_cpp's Makefile.incs in your Makefile
 - link with COMMON_OBJS, THROWS_OBJS, SCANNER_OBJS, LOGGER_OBJS and CPU_OBJS

Paths are modifiable, check Makefiles. For example code, clone this repository with
--recursive-submodules enabled.

## <sub>Example</sub>

Sample code for monitoring cpu usage and
single processe's resource usage is provided


