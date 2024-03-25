#include <iostream>
#include <chrono>
#include <thread>
#include <iomanip>
#include "cpu/cpu.hpp"
#include "cpu/process.hpp"

int main(int argc, char **argv) {

	std::cout << "cpu_cpp test" << std::endl;

	pid_t PID = ::getpid();

	cpu_t *cpu = new cpu_t();
	cpu_t::process_t *proc = new cpu_t::process_t(PID);

	std::cout << cpu << "\n" << std::endl;

	int i = 0;

	while ( i++ < 20 && !cpu -> disabled()) {
		std::this_thread::sleep_for (std::chrono::milliseconds(850));
		cpu -> update();
		proc -> update();
		std::cout << "cpu load: " << cpu -> load();
		std::cout << " process cpu usage: " << std::fixed << std::setprecision(2) << proc -> usage() << "%";
		std::cout << " RAM: " << proc -> memory_usage() << " kB on cpu: " << proc -> last_seen_on_cpu() << std::endl;
	}

	delete proc;
	delete cpu;
	return 0;
}
