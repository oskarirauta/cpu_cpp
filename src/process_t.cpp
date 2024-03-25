#include <string>
#include <sstream>
#include <istream>
#include <fstream>

#include "throws.hpp"
#include "scanner.hpp"
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

void parse(const std::string& s, const std::map<size_t, std::variant<int*, std::string*>>& m) {

	std::stringstream ss(s);
	size_t max_len = std::numeric_limits<std::streamsize>::max();
	size_t c = 0;

	for ( auto p : m ) {

		while ( c < p.first ) {
			ss.ignore(max_len, ' ');
			c++;
		}

		if ( std::holds_alternative<int*>(p.second))
			ss >> *std::get<int*>(p.second);

	}

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

int cpu_t::process_t::last_seen_on_cpu() const {
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

	unsigned long long new_utime, new_stime;

	size_t count = scan(line, {
			{ 13, &new_utime },
			{ 14, &new_stime },
			{ 22, &this -> _memory_usage },
			{ 38, &this -> _on_cpu }
		});

	if ( count < 2 )
		return;

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
