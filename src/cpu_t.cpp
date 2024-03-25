#include <filesystem>

#include "throws.hpp"
#include "logger.hpp"
#include "cpu/cpu.hpp"

cpu_t::cpu_t(int smoothing) {

	std::string line;
	std::ifstream fd("/proc/stat", std::ios::in);

	if ( fd.fail() || !fd.is_open() || !fd.good())
		throws << "failed to open /proc/stat" << std::endl;

	this -> _size = 0;
	this -> _smooth = 0;
	this -> _disabled = false;
	this -> _def_smooth = smoothing;
	this -> _temp_path = "-";
	this -> _temp = 0;

	while ( getline(fd, line)) {

		if ( line.starts_with("cpu") && !line.starts_with("cpu "))
			this -> _size++;

		update_load(line);
	}

	fd.close();
}

cpu_t::~cpu_t() {

	this -> nodes.clear();
}

int cpu_t::calculate_load(const cpu_t::node_t::tck_t& tck0, const cpu_t::node_t::tck_t& tck1) {

	unsigned long long total = tck1.total_ticks() - tck0.total_ticks();
	unsigned long long idle = tck1.idle_ticks() - tck0.idle_ticks();
	unsigned long long active = total - idle;
	double ret = active != 0 && total != 0 ? ( active * 100.f / total ) : (double)0;
	return ret > 100 ? 100 : ( ret < 0 ? 0 : (int)ret );
}

void cpu_t::update_load(const std::string& line) {

	if ( line.empty() || !line.starts_with("cpu"))
		return;

	else if ( line.starts_with("cpu ")) {

		this -> tck0 = this -> tck1;
		this -> tck1 = line;
		int result = this -> calculate_load(this -> tck0, this -> tck1);

		if ( result > 0 ) {
			this -> _smooth = this -> _def_smooth;
			this -> _load = result;
		} else if ( this -> _load > 0 && result == 0 && this -> _smooth > 0 ) {
			this -> _smooth--;
		} else if ( this -> _load > 0 && result == 0 && this -> _smooth == 0 ) {
			this -> _load = result;
		}

	} else if ( auto pos = line.find_first_of(' '); pos != std::string::npos ) {

		std::string name(line);
		name.erase(pos, name.size() - pos);
		name = common::trim_ws(common::to_lower(name));

		if ( name.empty() || !name.starts_with("cpu") || name.size() < 4 ) {
			logger::warning["cpu"] << "failed to parse cpu name from " << name << std::endl;
			return;
		}

		if ( !this -> nodes.contains(name))
			this -> nodes[name] = cpu_t::node_t(name);

		this -> nodes[name].tck0 = this -> nodes[name].tck1;
		this -> nodes[name].tck1 = line;
		int result = this -> calculate_load(this -> nodes[name].tck0, this -> nodes[name].tck1);

		if ( result > 0 ) {
			this -> nodes[name]._smooth = this -> _def_smooth;
			this -> nodes[name]._load = result;
		} else if ( this -> nodes[name]._load > 0 && result == 0 && this -> nodes[name]._smooth > 0 ) {
			this -> nodes[name]._smooth--;
		} else if ( this -> nodes[name]._load > 0 && result == 0 && this -> nodes[name]._smooth == 0 ) {
			this -> nodes[name]._load = result;
		}

	} else logger::warning["cpu"] << "failed to parse line " << line << std::endl;
}

static std::string init_cpu_temp() {

	std::filesystem::path basepath("/sys/class/thermal");

	if ( !std::filesystem::is_directory(basepath))
		return "";

	for ( auto const& dir_entry : std::filesystem::directory_iterator{basepath}) {

		if ( dir_entry.path().string().rfind(basepath.string() + "/thermal_", 0) != 0 )
			continue;

		std::filesystem::path typepath(dir_entry.path().string() + "/type");
		std::filesystem::path temppath(dir_entry.path().string() + "/temp");

		if ( !std::filesystem::exists(typepath) || !std::filesystem::exists(temppath) ||
			!std::filesystem::is_regular_file(typepath) || ! std::filesystem::is_regular_file(temppath))
			continue;

		std::fstream typefile(typepath.string(), std::ios::in);

		if ( !typefile.is_open() || !typefile.good()) {

			if ( typefile.is_open())
				typefile.close();
			continue;
		}

		std::fstream tempfile(temppath.string(), std::ios::in);

		if ( !tempfile.is_open() || !tempfile.good()) {

			if ( tempfile.is_open())
				tempfile.close();
			continue;
		}

		std::string z_type, z_temp;

		if ( std::getline(typefile, z_type) && std::getline(tempfile, z_temp) && z_type == "x86_pkg_temp" ) {

			typefile.close();
			tempfile.close();

			try {
				std::stoi(z_temp);
			} catch ( const std::exception& e ) {
				continue;
			}

			return temppath.string();

		} else {

			typefile.close();
			tempfile.close();
		}
	}

	return "";
}

int cpu_t::temp() {

	if ( this -> _temp_path == "-" )
		this -> _temp_path = init_cpu_temp();

	if ( this -> _temp_path.empty() || this -> _temp_path == "-" )
		return 0;

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

int cpu_t::load() {

	return this -> _load;
}

size_t cpu_t::size() {

	return this -> _size;
}

bool cpu_t::disabled() {

	return this -> _disabled;
}

void cpu_t::update() {

	if ( this -> _disabled )
		return;

	std::ifstream fd("/proc/stat", std::ios::in);

	if ( fd.fail() || !fd.is_open() || !fd.good()) {

		this -> _disabled = true;
		logger::error["cpu"] << "failed to read /proc/stat" << std::endl;
		if ( fd.is_open())
			fd.close();
		return;
	}

	std::string line;
	while ( getline(fd, line))
		update_load(line);

	fd.close();
}

std::string cpu_t::operator [](const std::string& name) {

	if ( this -> nodes.size() < 1 )
		throws << "no cpus detected" << std::endl;

	std::string _name = name;
	_name = common::trim_ws(common::to_lower(std::as_const(name)));

	if ( _name == "name" || _name == "cpu" )
		return "cpu";

	try {
		return this -> nodes.front().second[_name];
	} catch ( const std::runtime_error& e ) {
		throws << e.what() << std::endl;
	}
}

cpu_t::node_t& cpu_t::operator [](size_t i) {

	if ( this -> nodes.size() < i + 1 )
		throws << "cpu" << i << " out of bounds, only " << nodes.size() << " cpus were detected" << std::endl;

	std::string key = "cpu" + std::to_string(i);
	if ( !this -> nodes.contains(key))
		throws << key << " not found" << std::endl;

	return this -> nodes.at(key);
}

std::ostream& operator <<(std::ostream& os, cpu_t& cpu) {

	os << "cpus: " << cpu._size << "\n";
	os << "load: " << (int)cpu._load;

	for ( auto node : cpu.nodes )
		os << "\n\n" << node.second;
	return os;
}

std::ostream& operator <<(std::ostream& os, cpu_t *cpu) {

	os << *cpu;
	return os;
}
