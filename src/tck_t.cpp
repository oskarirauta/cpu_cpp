#include <utility>

#include "common.hpp"
#include "throws.hpp"
#include "cpu/cpu_t.hpp"

cpu_t::node_t::tck_t::tck_t(const std::string& s) {

	int i = 0;
	std::string id(common::trim_ws(std::as_const(s)) + " ");
	std::string word;

	while ( i < 11 && !id.empty()) {

		if ( common::whitespace.contains(id.front()) && word.empty()) {

			id.erase(0, 1);
			continue;
		} else if ( common::whitespace.contains(id.front())) {

			if ( i == 0 ) {
				word = "";
				i++;
				continue;
			}

			unsigned long long val;
			try {
				val = std::stoull(word);
			} catch ( const std::runtime_error& e ) {
				throws << "conversion error while converting " << word << " to number" << std::endl;
			}

			word = "";
			this -> operator[](i - 1) = val;
			i++;
			continue;
		}

		word += id.front();
		id.erase(0, 1);

	}
	//this -> operator[](i) = 5;

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
