#pragma once
#include "cpu/cpu.hpp"

class cpu_t::process_t {

	private:

		pid_t _pid;
		bool _good = true;
		int _on_cpu = 0;
		cpu_t::node_t::tck_t tck[2];
		unsigned long long utime[2], stime[2];
		unsigned long long _memory_usage = 0;
		double _usage = 0.0;

		std::string _cmdline;
		std::string _cmd;

		void update_cpu();
		void update_pid(const pid_t& pid);
		void update_args(const pid_t& pid);

	public:

		pid_t pid() const;
		std::string cmd() const;
		std::string cmdline() const;
		double usage() const;
		unsigned long long memory_usage() const;
		int last_seen_on_cpu() const;
		bool good() const;

		void update();

		process_t(const pid_t& pid);
};
