#include <string>
#include <sstream>
#include <istream>
#include <fstream>

#include "throws.hpp"
#include "cpu/cpu.hpp"
#include "cpu/process.hpp"

static std::string readline(const std::string& filename) {

	std::ifstream fd(filename);

	if ( fd.fail() || !fd.is_open() || !fd.good()) {

		if ( fd.is_open())
			fd.close();
		throws << "failed to open " << filename << std::endl;
	}

	std::string line;
	if ( !getline(fd, line)) {

		fd.close();
		throws << "failed to read " << filename << std::endl;
	}

	fd.close();
	return line;
}

pid_t cpu_t::process_t::pid() const {
	return this -> _pid;
}

std::string cpu_t::process_t::cmd() const {
	return this -> _cmd;
}

std::string cpu_t::process_t::cmdline() const {
	return this -> _cmdline;
}

bool cpu_t::process_t::good() const {
	return this -> _good;
}

double cpu_t::process_t::usage() const {
	return this -> _usage;
}

unsigned long long cpu_t::process_t::memory_usage() const {
	return this -> _memory_usage / 1024;
}

unsigned long long cpu_t::process_t::memory_available() const {
	return this -> _memory_avail;
}

int cpu_t::process_t::on_cpu() const {
	return this -> _on_cpu;
}

void cpu_t::process_t::update_cpu() {

	std::ifstream fd("/proc/stat", std::ios::in);

	if ( fd.fail() || !fd.is_open() || !fd.good()) {

		if ( fd.is_open())
			fd.close();
		throws << "failed to open /proc/stat" << std::endl;
	}

	std::string line;
	while ( getline(fd, line)) {

		if ( line.size() > 4 && line.rfind("cpu ", 0) == 0 )
			break;
		else line = "";
	}

	fd.close();

	if ( line.empty())
		throws << "failed to read /proc/stat" << std::endl;

	try {
		cpu_t::node_t::tck_t new_tck(line);
		this -> tck[0] = this -> tck[1];
		this -> tck[1] = new_tck;

	} catch ( const std::runtime_error& e ) {

		throws << "failed to parse /proc/stat" << std::endl;

	}
}

void cpu_t::process_t::update_pid(const pid_t& pid) {

	std::string line;

	try {
		line = readline("/proc/" + std::to_string(pid) + "/stat");
	} catch ( const std::runtime_error& e ) {
		throw e;
	}

	if ( line.empty())
		throws << "failed to parse /proc/" << pid << "/stat" << std::endl;

	std::stringstream ss(line);
	std::string ignored;
	unsigned long long new_utime, new_stime, mem;
	size_t max_length = std::numeric_limits<std::streamsize>::max();

	for ( int i = 0; i < 13; i++ )
		ss.ignore(max_length, ' ');

	ss >> new_utime >> new_stime;

	if ( !ss.good() || ss.fail() || ss.bad())
		throws << "failed to parse /proc/" << pid << "/stat" << std::endl;

	for ( int i = 0; i < 8; i++ )
		ss.ignore(max_length, ' ');

	ss >> mem;

	if ( ss.good() && !ss.fail() && !ss.bad())
		this -> _memory_usage = mem;

	this -> utime[0] = this -> utime[1];
	this -> stime[0] = this -> stime[1];
	this -> utime[1] = new_utime;
	this -> stime[1] = new_stime;
}

void cpu_t::process_t::update_args(const pid_t& pid) {

	std::string line;

	try {
		line = readline("/proc/" + std::to_string(pid) + "/comm");
	} catch ( const std::runtime_error& e ) {
		throw e;
	}

	while ( line.back() == 0 )
		line.pop_back();

	if ( line.empty())
		throws << "failed to parse /proc/" << pid << "/comm" << std::endl;

	this -> _cmd = line;
	line = "";

	try {
		line = readline("/proc/" + std::to_string(pid) + "/cmdline");
	} catch ( const std::runtime_error& e ) {
		throw e;
	}

	while ( line.back() == 0 )
		line.pop_back();

	if ( line.empty())
		throws << "failed to parse /proc/" << pid << "/cmdline" << std::endl;

	this -> _cmdline = line;

	std::ifstream fd("/proc/" + std::to_string(pid) + "/root/proc/meminfo", std::ios::in);

	if ( fd.fail() || !fd.is_open() || !fd.good()) {

		fd = std::ifstream("/proc/meminfo", std::ios::in);

		if (( fd.fail() || !fd.good()) && fd.is_open())
				fd.close();
	}

	if ( fd.is_open()) {

		line = "";
		while ( getline(fd, line)) {

			if ( line.size() > 10 && line.rfind("MemTotal:", 0) == 0 )
				break;
			else line = "";
		}

		fd.close();

		if ( !line.empty()) {

			std::stringstream ss(line);
			unsigned long long mem;

			ss.ignore(std::numeric_limits<std::streamsize>::max(), ' ');
			ss >> mem;

			if ( ss.good() && !ss.fail() && !ss.bad())
				this -> _memory_avail = mem;
		}
	}

}

void cpu_t::process_t::update() {

	this -> _good = true;

	try {
		this -> update_cpu();
	} catch ( const std::runtime_error& e ) {
		this -> _good = false;
		throw e;
	}

	try {
		this -> update_pid(this -> _pid);
	} catch ( const std::runtime_error& e ) {
		this -> _good = false;
		throw e;
	}

	if ( this -> _good && !this -> tck[0].empty() && !this -> tck[1].empty()) {

		unsigned long long time_diff = this -> tck[1].total_ticks() - this -> tck[0].total_ticks();

		double usr = 100.0f * ( this -> utime[1] - this -> utime[0] ) / time_diff;
		double sys = 100.0f * ( this -> stime[1] - this -> stime[0] ) / time_diff;
		this -> _usage = usr + sys;
	}
}

cpu_t::process_t::process_t(const pid_t& pid) {

	this -> _pid = pid;
	this -> _good = true;

	try {
		this -> update_args(pid);
	} catch ( const std::runtime_error& e ) {
		this -> _good = false;
		throw e;
	}

	try {
		this -> update_cpu();
	} catch ( const std::runtime_error& e ) {
		this -> _good = false;
		throw e;
	}

	try {
		this -> update_pid(pid);
	} catch ( const std::runtime_error& e ) {
		this -> _good = false;
		throw e;
	}

}
