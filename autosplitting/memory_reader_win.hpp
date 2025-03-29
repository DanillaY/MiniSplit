#include <Windows.h>
#include <iostream>
#include <psapi.h>
#include <string>
#include <type_traits>
#include <cstdint>
#include "memory_reader_base_win.hpp"

#pragma once

//adds the offset address to the given address and returns new address with the applied offset, use add_offset32 if the process is 32 bit
uintptr_t add_offset32(HANDLE handle, uintptr_t base, uintptr_t offset) {
    uintptr_t pointer1_address = base + offset;
    uint32_t new_pointer1;
    ReadProcessMemory(handle, (LPCVOID)pointer1_address, &new_pointer1, sizeof(new_pointer1), nullptr);

    return new_pointer1;
}

//adds the offset address to the given address and returns new address with the applied offset, use add_offset64 if the process is 64 bit
uintptr_t add_offset64(HANDLE handle, uintptr_t base, uintptr_t offset) {
    uintptr_t pointer1_address = base + offset;
    uint64_t new_pointer1;
    ReadProcessMemory(handle, (LPCVOID)pointer1_address, &new_pointer1, sizeof(new_pointer1), nullptr);

    return new_pointer1;
}

uintptr_t add_all_offsets(HANDLE handle, uintptr_t base_module, uintptr_t offsets[], unsigned int offsets_len) {
    bool process64 = is_64bit_process(handle);
    uintptr_t base_and_offsets =  process64 ? add_offset64(handle,base_module,offsets[0]): add_offset32(handle,base_module,offsets[0]);
    
    //we start i at 1 because we added base address and the first offset already
    for(int i = 1; i < offsets_len;i++) {

        //we want to add all offsets except the last one because it will be dereferenced as a value in read_proc_memory function
        if(i+1 <= offsets_len) {
            base_and_offsets =  process64 ? add_offset64(handle,base_and_offsets,offsets[i]): add_offset32(handle,base_and_offsets,offsets[i]);
        }
    }
   
    uintptr_t pointer_with_offsets = base_and_offsets + offsets[offsets_len];
    return pointer_with_offsets;
}

//use read_proc_memory to read any basic buffer types
//template <typename T>
int read_proc_memory(int pid, uintptr_t base_module , uintptr_t offsets[], unsigned int offsets_len, int buffer) {

    HANDLE handle = OpenProcess(PROCESS_VM_READ | PROCESS_VM_OPERATION |PROCESS_QUERY_INFORMATION, FALSE, pid);
	auto last_pointer = offsets_len > 1 ? add_all_offsets(handle,base_module, offsets, offsets_len) : base_module+offsets[0];
    
	ReadProcessMemory(handle, LPCVOID(last_pointer), &buffer, sizeof(buffer), nullptr);

    return buffer;
}

//use read_proc_memory_string to read a char* type
char* read_proc_memory_string(int pid, uintptr_t base_module , uintptr_t offsets[], unsigned int offsets_len, char* buffer, size_t buffer_size) {

    HANDLE handle = OpenProcess(PROCESS_VM_READ | PROCESS_VM_OPERATION |PROCESS_QUERY_INFORMATION, FALSE, pid);
    auto last_pointer = offsets_len > 1 ? add_all_offsets(handle,base_module, offsets, offsets_len) : base_module+offsets[0];

    if (ReadProcessMemory(handle, LPCVOID(last_pointer), buffer, buffer_size, nullptr)) {
        std::cout << "Value: " << buffer << std::endl;
    } else {
        buffer[0] = '\0';
    }
	return buffer;
}