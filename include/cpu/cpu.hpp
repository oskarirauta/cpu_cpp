#pragma once

#include <string>
#include <memory>
#include <vector>
#include <fstream>
#include "common.hpp"
#include "lowercase_map.hpp"

class cpu_t {

	public:
		class process_t;
		friend class process_t;

		class node_t {

			friend class cpu_t;
			friend class cpu_t::process_t;

			private:

				class tck_t {

					friend class cpu_t;
					friend class cpu_t::node_t;
					friend class cpu_t::process_t;

					private:

						unsigned long long user;
						unsigned long long nice;
						unsigned long long system;
						unsigned long long idle;
						unsigned long long iowait;
						unsigned long long irq;
						unsigned long long softirq;
						unsigned long long steal;
						unsigned long long guest;
						unsigned long long guest_nice;

						unsigned long long& operator [](const int i);

					public:

						unsigned long long total_ticks() const;
						unsigned long long idle_ticks() const;
						unsigned long long busy_ticks() const;
						bool empty() const;

						tck_t& operator =(const tck_t& other);
						tck_t& operator =(const std::string& s);
						bool operator ==(const tck_t& other);
						bool operator !=(const tck_t& other);

						tck_t() : user(0), nice(0), system(0), idle(0), iowait(0), irq(0), softirq(0), steal(0), guest(0), guest_nice(0) {}
						tck_t(const tck_t& other) : user(other.user), nice(other.nice), idle(other.idle), iowait(other.iowait), irq(other.irq),
									softirq(other.softirq), steal(other.steal), guest(other.guest), guest_nice(other.guest_nice) {}
						tck_t(const std::string& s);
				};

				std::string _id;
				int _number;
				int _load;
				int _smooth;
				int _core;
				int _temp;
				int _temp_max;
				std::string _temp_path;
				cpu_t::node_t::tck_t tck0, tck1;
				common::lowercase_map<std::string> values;

			public:

				std::string id() const;
				int number() const;
				int core() const;
				int load() const;
				int temp() const;
				int temp();
				int temp_max() const;
				std::string temp_file() const;

				std::string operator [](const std::string& name) const;

				unsigned long long user() const;
				unsigned long long nice() const;
				unsigned long long system() const;
				unsigned long long idle() const;
				unsigned long long iowait() const;
				unsigned long long irq() const;
				unsigned long long softirq() const;
				unsigned long long steal() const;
				unsigned long long guest() const;
				unsigned long long guest_nice() const;

				unsigned long long total_ticks() const;
				unsigned long long idle_ticks() const;
				unsigned long long busy_ticks() const;

				node_t() : _id(""), _load(0), _smooth(0), _core(0), _temp(-1), _temp_path("-") {};
				node_t(const std::string& id);

				friend std::ostream& operator <<(std::ostream& os, const node_t& node);

		};

		class iterator {

			friend class cpu_t;

			private:
				common::lowercase_map<cpu_t::node_t>::iterator _it;
				iterator(const common::lowercase_map<cpu_t::node_t>::iterator& it) : _it(it) {}

			public:
				iterator() = default;
				iterator& operator++() { ++this -> _it; return *this; }
				iterator operator++(int) { auto tmp = *this; ++(*this); return tmp; }
				bool operator==(const iterator& rhs) { return this -> _it == rhs._it; }
				bool operator!=(const iterator& rhs) { return this -> _it != rhs._it; }
				const cpu_t::node_t& operator*() { return this -> _it.operator -> () -> second; }
				const cpu_t::node_t* operator->() { return &this -> _it.operator -> () -> second; }
		};

		iterator begin();
		iterator end();

		int temp();
		int temp() const;
		int temp_max() const;

		int cores() const;
		int load() const;
		size_t size() const;
		bool disabled() const;
		std::string temp_file() const;

		void update();
		std::string operator [](const std::string& name);
		cpu_t::node_t& operator [](size_t i);

		cpu_t(int smoothing = 1);
		~cpu_t();

	private:

		bool _disabled;
		size_t _size;
		int _load;
		int _smooth;
		int _def_smooth;
		int _temp;
		int _temp_max;
		int _cores;
		cpu_t::node_t::tck_t tck0, tck1;
		common::lowercase_map<cpu_t::node_t> nodes;
		std::string _temp_path;

		void update_load(const std::string& line);
		int calculate_load(const cpu_t::node_t::tck_t& tck0, const cpu_t::node_t::tck_t& tck1);

		friend std::ostream& operator <<(std::ostream& os, const cpu_t& cpu);
		friend std::ostream& operator <<(std::ostream& os, cpu_t *cpu);
};

std::ostream& operator <<(std::ostream& os, const cpu_t::node_t& node);
std::ostream& operator <<(std::ostream& os, const cpu_t& cpu);
std::ostream& operator <<(std::ostream& os, cpu_t *cpu);
