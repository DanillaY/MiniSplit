#include <cstdint>
#include <string>

#pragma once

class Basic_Process_Info {

	private:
		char* process_name;
        uintptr_t base_module_address;
        int pid;

		static Basic_Process_Info* bpi;

		Basic_Process_Info (char* process_name, uintptr_t base_module_address, int pid) {
            this->process_name = process_name;
            this->base_module_address = base_module_address;
            this->pid = pid;
        }

    public:

		Basic_Process_Info(const Basic_Process_Info&) = delete;
    	Basic_Process_Info& operator=(const Basic_Process_Info&) = delete;
		
		static Basic_Process_Info& get_instance(char* process_name, uintptr_t base_module_address, int pid) {

			if (bpi == nullptr) {
				bpi = new Basic_Process_Info(process_name, base_module_address, pid);
			}
			
			return *bpi;
		}

		void set_base_module_address(std::mutex& mutex, uintptr_t base_module_address){
			std::lock_guard lock(mutex);
			this->base_module_address= base_module_address;
		}

		void set_pid(std::mutex& mutex, int pid) {
			std::lock_guard lock(mutex);
			this->pid = pid;
		}

		char* get_process_name() {return process_name;}
		uintptr_t get_base_offset() {return base_module_address;}
		int get_pid() {return pid;}
};