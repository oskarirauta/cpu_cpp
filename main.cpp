#include <iostream>
#include <chrono>
#include <thread>
#include "cpu/cpu.hpp"

int main(int argc, char **argv) {

	std::cout << "cpu_cpp test" << std::endl;

	//cpu_t::tck_t x("cpu  4830396 21 862783 5044180904 42128 0 316560 0 0 0");

	cpu_t *cpu = new cpu_t();

	std::cout << cpu << "\n" << std::endl;

	int i = 0;

	while ( i++ < 20 && !cpu -> disabled()) {
		std::this_thread::sleep_for (std::chrono::milliseconds(850));
		cpu -> update();
		std::cout << "cpu load: " << cpu -> load() << std::endl;
	}

	delete cpu;
	return 0;
}
