#include <iostream>
#include <cmath>

#include "throws.hpp"
#include "common.hpp"
#include "cpu/cpu.hpp"

static std::string only_numbers(const std::string& s) {

	std::string _s(s);
	auto pos = _s.find_first_not_of("1234567890");
	while ( pos != std::string::npos ) {
		_s.erase(pos, 1);
		pos = _s.find_first_not_of("1234567890");
	}
	return _s;
}

static std::string no_decimals(const std::string& s) {

	std::string _s(s);
	while ( _s.back() == '.' )
		_s.pop_back();

	if ( auto pos = _s.find_first_of("."); pos != std::string::npos )
		_s.erase(pos, _s.size() - pos);
	return _s;
}

static std::string rounded(const std::string& s) {

	double d;
	try {
		d = std::stod(common::trim_ws(std::as_const(s)));
	} catch ( const std::runtime_error& e ) {
		return no_decimals(s);
	}
	return std::to_string((int)::round(d));
}

unsigned long long cpu_t::node_t::user() const {
	return this -> tck1.user;
}

unsigned long long cpu_t::node_t::nice() const {
	return this -> tck1.nice;
}

unsigned long long cpu_t::node_t::system() const {
	return this -> tck1.system;
}

unsigned long long cpu_t::node_t::idle() const {
	return this -> tck1.idle;
}

unsigned long long cpu_t::node_t::iowait() const {
	return this -> tck1.iowait;
}

unsigned long long cpu_t::node_t::irq() const {
	return this -> tck1.irq;
}

unsigned long long cpu_t::node_t::softirq() const {
	return this -> tck1.softirq;
}

unsigned long long cpu_t::node_t::steal() const {
	return this -> tck1.steal;
}

unsigned long long cpu_t::node_t::guest() const {
	return this -> tck1.guest;
}

unsigned long long cpu_t::node_t::guest_nice() const {
	return this -> tck1.guest_nice;
}

unsigned long long cpu_t::node_t::total_ticks() const {
	return this -> tck1.total_ticks();
}

unsigned long long cpu_t::node_t::idle_ticks() const {
	return this -> tck1.idle_ticks();
}

unsigned long long cpu_t::node_t::busy_ticks() const {
	return this -> tck1.busy_ticks();
}

cpu_t::node_t::node_t(const std::string& id) {

	if ( id.size() < 4 )
		throws << "invalid cpu name " << id << std::endl;

	this -> _id = id;
	std::string s = id;
	s.erase(0, 3);

	if ( s.find_first_not_of("1234567890") != std::string::npos )
		throws << "invalid cpu name " << id << ", cpu number " << s << " contains illegal characters, only numbers are valid" << std::endl;

	try {
		this -> _number = std::stoi(s);
	} catch ( const std::runtime_error& e ) {
		throws << "cpu " << id << " number could not be parsed from " << s << std::endl;
	}

	std::ifstream fd("/proc/cpuinfo", std::ios::in);

	if ( fd.fail() || !fd.is_open() || !fd.good())
		throws << "failed to open /proc/cpuinfo" << std::endl;

	std::string line;
	std::string cpuname;

	while ( std::getline(fd, line)) {

		while ( line.back() == ':' )
			line.pop_back();

		if ( auto pos = line.find_first_of(":"); pos != std::string::npos ) {

			std::string key = line;
			std::string value = line;

			key.erase(pos, key.size() - pos);
			value.erase(0, pos + 1);

			key = common::trim_ws(common::to_lower(key));
			value = common::trim_ws(value);

			if ( key == "processor" ) {
				cpuname = value;
				if ( value == s )
					this -> values.append({ "name", id });
			}

			if ( cpuname != s )
				continue;

			if ( key == "vendor_id" ) key = "vendor";
			else if ( key == "cpu family" ) key = "family";
			else if ( key == "model name" ) key = "model";
			else if ( key == "cpu mhz" ) key = "mhz";
			else if ( key == "cache size" ) key = "cache";
			else if ( key == "cpu cores" ) key = "cores";
			else if ( key != "stepping" && key != "microcode" && key != "fpu" &&
				key != "bogomips" && key != "cache_alignment" )
				continue;

			if ( key == "cache" ) {
				value = only_numbers(value);
				value = common::trim_ws(value);
			} else if ( key == "mhz" || key == "bogomips" )
				value = rounded(value);

			if ( key.empty() || value.empty() || this -> values.contains(key))
				continue;

			this -> values.append({ key, value });
		}
	}

	fd.close();
}

std::string cpu_t::node_t::id() const {

	return this -> _id;
}

int cpu_t::node_t::load() const {

	return this -> _load;
}

std::string cpu_t::node_t::operator [](const std::string& name) const {

	std::string s = common::trim_ws(common::to_lower(std::as_const(name)));

	if ( !this -> values.contains(s))
		throws << "unknown key " << s << std::endl;

	for ( auto value : this -> values )
		if ( value.first == s )
			return value.second;

	throws << "unknown error while retrieving " << this -> _id << "[" << s << "]" << std::endl;
}

std::ostream& operator <<(std::ostream& os, const cpu_t::node_t& node) {

	for ( auto value : node.values )
		os << value.first << ": " << value.second << "\n";
	os << "load: " << (int)node._load;
	return os;
}
