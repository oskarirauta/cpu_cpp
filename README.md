[![License:MIT](https://img.shields.io/badge/License-MIT-blue?style=plastic)](LICENSE)
[![C++ CI build](../../actions/workflows/build.yml/badge.svg)](../../actions/workflows/build.yml)

### cpu_cpp

CPU and per-process monitoring library for Linux (reads `/proc/stat`,
`/proc/cpuinfo` and `/proc/<pid>/*`).

## <sub>Usage</sub>

```cpp
#include <iostream>
#include <thread>
#include <chrono>
#include <unistd.h>
#include "cpu/cpu.hpp"
#include "cpu/process.hpp"

int main() {

    cpu_t cpu;                          // reads /proc/stat + /proc/cpuinfo
    cpu_t::process_t self(::getpid());

    for ( int i = 0; i < 5 && !cpu.disabled(); i++ ) {

        std::this_thread::sleep_for(std::chrono::seconds(1));
        cpu.update();
        self.update();

        std::cout << "load " << cpu.load() << "%  temp " << cpu.temp() << "C"
                  << "  (this process: " << self.usage() << "% cpu, "
                  << self.memory_usage() << " kB)\n";
    }
    return 0;
}
```

`cpu.load()` is the overall busy percentage and `cpu.temp()` the package
temperature (or `-1` when no sensor is found); `cpu.cores()` / `cpu.size()` give
the physical-core and logical-cpu counts. Iterate the per-cpu nodes with
`for ( const auto& node : cpu )` - each `node` exposes `id()`, `load()`,
`core()`, `temp()`, and any `/proc/cpuinfo` field via `node["model"]`,
`node["bogomips"]`, ...

`cpu_t::process_t` tracks a single pid: `usage()` (cpu %), `memory_usage()`
(kB), `last_seen_on_cpu()`, `cmd()` and `cmdline()`.

## <sub>Dependencies</sub>

 - [common_cpp](https://github.com/oskarirauta/common_cpp.git)
 - [throws_cpp](https://github.com/oskarirauta/throws_cpp.git)
 - [logger_cpp](https://github.com/oskarirauta/logger_cpp.git)

## <sub>Importing</sub>

 - clone common_cpp, throws_cpp and logger_cpp into the `common`, `throws` and
   `logger` sub directories, and cpu_cpp into `cpu`
 - include each library's `Makefile.inc` in your Makefile
 - link with `$(COMMON_OBJS)`, `$(THROWS_OBJS)`, `$(LOGGER_OBJS)` and `$(CPU_OBJS)`

Paths are configurable, check the Makefiles. For the example, clone this
repository with `--recurse-submodules`.

## <sub>Example</sub>

Runnable example code is in [`main.cpp`](main.cpp).
