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

uintptr_t add_all_offsets(HANDLE handle, uintptr_t base_module, std::vector<uintptr_t> offsets, unsigned int offsets_len) {
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

int read_proc_memory(int pid, uintptr_t base_module , std::vector<uintptr_t> offsets, unsigned int offsets_len, int buffer) {

    HANDLE handle = OpenProcess(PROCESS_VM_READ | PROCESS_VM_OPERATION |PROCESS_QUERY_INFORMATION, FALSE, pid);
    auto last_pointer = offsets_len > 1 ? add_all_offsets(handle,base_module, offsets, offsets_len) : base_module+offsets[0];

    ReadProcessMemory(handle, LPCVOID(last_pointer), &buffer, sizeof(buffer), nullptr);

    return buffer;
}

//use read_proc_memory_string to read a char* type
auto read_proc_memory_c_string(int pid, uintptr_t base_module , std::vector<uintptr_t>offsets, unsigned int offsets_len, char buffer[], size_t buffer_size) {

    HANDLE handle = OpenProcess(PROCESS_VM_READ | PROCESS_VM_OPERATION |PROCESS_QUERY_INFORMATION, FALSE, pid);
    auto last_pointer = offsets_len >= 1 ? add_all_offsets(handle,base_module, offsets, offsets_len) : base_module+offsets[0];

    ReadProcessMemory(handle, LPCVOID(last_pointer), buffer, buffer_size, nullptr);
    return buffer;
}


//use read_proc_memory_string_unknown_size when the buffer size is not determined or it could vary, the buffer_size that this funcrion gets is a default value (most of the time its minimal size of the string in the set)
auto read_proc_memory_string_unknown_size(int pid, uintptr_t base_module , std::vector<uintptr_t>offsets, unsigned int offsets_len,std::string buffer, size_t buffer_size) {

    HANDLE handle = OpenProcess(PROCESS_VM_READ | PROCESS_VM_OPERATION |PROCESS_QUERY_INFORMATION, FALSE, pid);
    auto last_pointer = offsets_len >= 1 ? add_all_offsets(handle,base_module, offsets, offsets_len) : base_module+offsets[0];

    buffer.resize(buffer_size);

    while(true) {
        size_t buffer_read = 0;

        if (ReadProcessMemory(handle, LPCVOID(last_pointer), &buffer[0], buffer_size, &buffer_read) == false) {
            buffer.clear();
            return buffer;
        }

        size_t null_pos = buffer.find('\0');
        if(null_pos != std::string::npos) {
            buffer.resize(null_pos);
            return buffer;
        }

        buffer_size *=2;
        buffer.resize(buffer_size);
    }
    
    return buffer;

}