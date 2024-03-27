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

static std::string init_core_temp(int core, int* temp_max) {

	if ( std::filesystem::is_directory(std::filesystem::path("/sys/devices/platform/coretemp.0/hwmon/hwmon1"))) {

		std::filesystem::path basepath("/sys/devices/platform/coretemp.0/hwmon/hwmon1");
		size_t basepath_size = basepath.string().size();

		for ( auto const& dir_entry : std::filesystem::directory_iterator{basepath}) {

			if ( dir_entry.path().string().rfind(basepath.string() + "/temp", 0) != 0 ||
				!dir_entry.path().string().ends_with("_label"))
				continue;

			std::string basename = "";
			size_t direntry_size = dir_entry.path().string().size();

			if ( auto pos = dir_entry.path().string().find_first_of("_"); pos != std::string::npos &&
				dir_entry.path().string().size() > basepath.string().size() + 1 ) {

				basename = dir_entry.path().string().substr(basepath_size, direntry_size - basepath_size);

				pos -= basepath_size;

				while ( basename.front() == '/' ) {
					basename.erase(0, 1);
					pos -= 1;
				}

				basename = basename.substr(0, pos);
				if ( basename.back() != '_' )
					basename += '_';

				std::filesystem::path label(basepath.string() + "/" + basename + "label");
				std::filesystem::path input(basepath.string() + "/" + basename + "input");
				std::filesystem::path crit(basepath.string() + "/" + basename + "crit");

				if ( !basename.starts_with("temp") ||
					!std::filesystem::exists(label) || !std::filesystem::exists(input))
					continue;

				std::fstream label_file(label.string(), std::ios::in);

				if ( !label_file.is_open() || !label_file.good()) {

					if ( label_file.is_open())
						label_file.close();

					continue;
				}

				std::string line;
				if ( !std::getline(label_file, line)) {

					label_file.close();
					continue;

				} else line = common::to_lower(line);

				label_file.close();

				if ( line.empty() || common::to_lower(line) != std::string("core " + std::to_string(core)))
					continue;

				std::fstream input_file(input.string(), std::ios::in);

				if ( !input_file.is_open() || !input_file.good()) {

					if ( input_file.is_open())
						input_file.close();

					continue;
				}

				if ( !std::getline(input_file, line) || line.empty() || line.find_first_not_of("1234567890") != std::string::npos ) {

					input_file.close();
					continue;
				}

				input_file.close();

				try {
					std::stoi(line);
				} catch ( const std::exception& e ) {
					continue;
				}

				if ( temp_max != nullptr && std::filesystem::exists(crit)) {

					std::fstream crit_file(crit.string(), std::ios::in);

					if ( crit_file.is_open() && crit_file.good()) {

						if ( std::getline(crit_file, line) && !line.empty() && line.find_first_not_of("1234567890") == std::string::npos ) {

							try {
								*temp_max = std::stoi(line);
								*temp_max *= 0.001;

							} catch ( const std::exception& e ) {

								*temp_max = -1;
							}
						}

					} else *temp_max = -1;

					if ( crit_file.is_open())
						crit_file.close();
				}


				return input.string();

			} else continue;
		}
	}

	return "";
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

	this -> _temp_path = "-";
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
			else if ( key == "core id" ) key = "core";
			else if ( key != "stepping" && key != "microcode" && key != "fpu" &&
				key != "bogomips" && key != "cache_alignment" )
				continue;

			if ( key == "cache" ) {
				value = only_numbers(value);
				value = common::trim_ws(value);
			} else if ( key == "core" ) {

				value = only_numbers(value);
				value = common::trim_ws(value);

				if ( value.empty() || value.find_first_not_of("1234567890") != std::string::npos )
					value = "0";

				try {
					this -> _core = std::stoi(value);
				} catch ( const std::exception& e ) {
					this -> _core = 0;
				}

			} else if ( key == "mhz" || key == "bogomips" )
				value = rounded(value);

			if ( key.empty() || value.empty() || this -> values.contains(key))
				continue;

			this -> values.append({ key, value });
		}
	}

	if ( !this -> values.contains("core")) {

		this -> values.append({ "core", "0" });
		this -> _core = 0;
	}

	fd.close();

	this -> _temp_path = init_core_temp(this -> _core, &this -> _temp_max);
	this -> temp();
}

std::string cpu_t::node_t::id() const {

	return this -> _id;
}

int cpu_t::node_t::core() const {

	return this -> _core;
}

int cpu_t::node_t::load() const {

	return this -> _load;
}

std::string cpu_t::node_t::temp_file() const {

	return this -> _temp_path;
}

int cpu_t::node_t::temp_max() const {

	return this -> _temp_max;
}

int cpu_t::node_t::temp() {

	if ( this -> _temp_path == "-" )
		this -> _temp_path = init_core_temp(this -> _core, &this -> _temp_max);

	if ( this -> _temp_path.empty() || this -> _temp_path == "-" )
		return -1;

	std::fstream tempfile(this -> _temp_path, std::ios::in);

	if ( !tempfile.is_open())
		return this -> _temp;
	else if ( !tempfile.good())
		return this -> _temp;

	std::string line;
	if ( std::getline(tempfile, line)) {

		try {
			int t = std::stoi(line);
			this -> _temp = t * 0.001;

		} catch ( const std::exception& e ) { }
	}

	tempfile.close();
	return this -> _temp;
}

int cpu_t::node_t::temp() const {

	if ( this -> _temp_path == "-" )
		return this -> _temp;

	if ( this -> _temp_path.empty() || this -> _temp_path == "-" )
		return this -> _temp;

	std::fstream tempfile(this -> _temp_path, std::ios::in);

	if ( !tempfile.is_open())
		return this -> _temp;
	else if ( !tempfile.good())
		return this -> _temp;

	std::string line;
	int _temp;
	if ( std::getline(tempfile, line)) {

		try {
			int t = std::stoi(line);
			_temp = t * 0.001;
		} catch ( const std::exception& e ) {
			_temp = this -> _temp;
		}
	}

	tempfile.close();
	return _temp;
}

std::string cpu_t::node_t::operator [](const std::string& name) const {

	std::string s = common::trim_ws(common::to_lower(std::as_const(name)));

	if ( !this -> values.contains(s))
		throws << "unknown key " << s << std::endl;

	for ( auto value : this -> values )
		if ( value.first == s )
			return value.second;

	if ( s == "temp" || s == "temperature" )
		return std::to_string(this -> temp());

	if (( s.starts_with("temperature") && ( s.ends_with("max") || s.ends_with("maximum"))) ||
		( s.ends_with("temperature") && ( s.starts_with("max") || s.starts_with("maximum"))))
		return std::to_string(this -> temp_max());

	throws << "unknown error while retrieving " << this -> _id << "[" << s << "]" << std::endl;
}

std::ostream& operator <<(std::ostream& os, const cpu_t::node_t& node) {

	for ( auto value : node.values )
		os << value.first << ": " << value.second << "\n";
	os << "load: " << (int)node._load << "\ntemp: " << node.temp() << " max: " << node.temp_max();
	return os;
}
