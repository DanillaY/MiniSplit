#include <cstdint>

#pragma once

class Basic_Process_Info {

    public:

        char* process_name;
        uintptr_t base_module_address;
        int pid;
    
        Basic_Process_Info (char* process_name, uintptr_t base_module_address, int pid) {
            this->process_name = process_name;
            this->base_module_address = base_module_address;
            this->pid = pid;
        }
};