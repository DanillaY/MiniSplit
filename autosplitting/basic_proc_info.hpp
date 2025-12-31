#include <cstdint>
#include <mutex>

#pragma once

class Basic_Process_Info {

    private:
        char* process_name;
        uintptr_t base_module_address;
        int pid;
        bool is_64bit;

        static Basic_Process_Info* bpi;

        Basic_Process_Info (char* process_name, uintptr_t base_module_address, int pid, bool is_64bit) {
            this->process_name = process_name;
            this->base_module_address = base_module_address;
            this->pid = pid;
            this->is_64bit = is_64bit;
        }

    public:

        Basic_Process_Info(const Basic_Process_Info&) = delete;
        Basic_Process_Info& operator=(const Basic_Process_Info&) = delete;
        
        static Basic_Process_Info& get_instance(char* process_name, uintptr_t base_module_address, int pid, bool is_64bit) {

            if (bpi == nullptr) {
                bpi = new Basic_Process_Info(process_name, base_module_address, pid, is_64bit);
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

        void set_is_64bit(std::mutex& mutex, bool is_64bit) {
            std::lock_guard lock(mutex);
            this->is_64bit = is_64bit;
        }

        char* get_process_name() {return process_name;}
        uintptr_t get_base_offset() {return base_module_address;}
        int get_pid() {return pid;}
        bool get_is_64bit() {return is_64bit;}
};