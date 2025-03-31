#include <cstdint>
#include <string>

#pragma once

class Basic_Process_Info {

	private:
		std::string process_name;
        uintptr_t base_module_address;
        int pid;

    public:

        Basic_Process_Info (char* process_name, uintptr_t base_module_address, int pid) {
            this->process_name = process_name;
            this->base_module_address = base_module_address;
            this->pid = pid;
        }

		std::string get_process_name(std::mutex& mutex) {
			std::lock_guard lock(mutex);
			return process_name;
		}

		uintptr_t get_base_module_address(std::mutex& mutex) {
			std::lock_guard lock(mutex);
			return base_module_address;
		}

		int get_pid(std::mutex& mutex) {
			std::lock_guard lock(mutex);
			return pid;
		}

		void set_process_name(std::mutex& mutex, char* process_name) {
			std::lock_guard lock(mutex);
			this->process_name = process_name;
		}

		void set_base_module_address(std::mutex& mutex, uintptr_t base_module_address){
			std::lock_guard lock(mutex);
			this->base_module_address= base_module_address;
		}

		void set_pid(std::mutex& mutex, int pid) {
			std::lock_guard lock(mutex);
			this->pid = pid;
		}
};