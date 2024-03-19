#include <utility>
#include <istream>
#include <sstream>

#include "common.hpp"
#include "throws.hpp"
#include "cpu/cpu.hpp"

cpu_t::node_t::tck_t::tck_t(const std::string& s) {

	std::istringstream ss(s + ( std::isspace(s.back()) ? "" : "\n"));
	std::string cpu_id;

	ss >> cpu_id >>
		this -> user >> this -> nice >> this -> system >> this -> idle >>
		this -> iowait >> this -> irq >> this -> softirq >> this -> steal >>
		this -> guest >> this -> guest_nice;

	if ( !ss.good() || ss.fail() || ss.bad())
		throws << "conversion error while parsing cpu stats from " << s << std::endl;
}

unsigned long long cpu_t::node_t::tck_t::total_ticks() const {

	return this -> user +
		this -> nice +
		this -> system +
		this -> idle +
		this -> iowait +
		this -> irq +
		this -> softirq +
		this -> steal +
		this -> guest +
		this -> guest_nice;
}

unsigned long long cpu_t::node_t::tck_t::idle_ticks() const {

	return this -> idle + this -> iowait;
}

unsigned long long cpu_t::node_t::tck_t::busy_ticks() const {

	return this -> total_ticks() - this -> idle_ticks();
}

bool cpu_t::node_t::tck_t::empty() const {

	return this -> user == 0 && this -> nice == 0 && this -> system == 0 && this -> idle == 0 &&
		this -> iowait == 0 && this -> irq == 0 && this -> softirq == 0 && this -> steal == 0 &&
		this -> guest == 0 && this -> guest_nice == 0;
}

cpu_t::node_t::tck_t& cpu_t::node_t::tck_t::operator =(const cpu_t::node_t::tck_t& other) {

	this -> user = other.user;
	this -> nice = other.nice;
	this -> system = other.system;
	this -> idle = other.idle;
	this -> iowait = other.iowait;
	this -> irq = other.irq;
	this -> softirq = other.softirq;
	this -> steal = other.steal;
	this -> guest = other.guest;
	this -> guest_nice = other.guest_nice;

	return *this;
}

cpu_t::node_t::tck_t& cpu_t::node_t::tck_t::operator =(const std::string& s) {

	tck_t other(s);
	this -> user = other.user;
	this -> nice = other.nice;
	this -> system = other.system;
	this -> idle = other.idle;
	this -> iowait = other.iowait;
	this -> irq = other.irq;
	this -> softirq = other.softirq;
	this -> steal = other.steal;
	this -> guest = other.guest;
	this -> guest_nice = other.guest_nice;

	return *this;
}

bool cpu_t::node_t::tck_t::operator ==(const tck_t& other) {

	return this -> user == other.user && this -> nice == other.nice && this -> system == other.system &&
		this -> idle == other.idle && this -> iowait == other.iowait && this -> irq == other.irq &&
		this -> softirq == other.softirq && this -> steal == other.steal &&
		this -> guest == other.guest && this -> guest_nice == other.guest_nice;
}

bool cpu_t::node_t::tck_t::operator !=(const tck_t& other) {

	return !(this -> operator ==(other));
}

unsigned long long& cpu_t::node_t::tck_t::operator [](const int i) {

	switch ( i ) {
		case 0: return this -> user;
		case 1: return this -> nice;
		case 2: return this -> system;
		case 3: return this -> idle;
		case 4: return this -> iowait;
		case 5: return this -> irq;
		case 6: return this -> softirq;
		case 7: return this -> steal;
		case 8: return this -> guest;
		case 9: return this -> guest_nice;
		default:
			throws << "outside of operator[] range 0-9" << std::endl;
	}

}
