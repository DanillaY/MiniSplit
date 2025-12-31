#include <cstdint>
#include <cstdio>
#include <sched.h>
#include <string>
#include <sys/types.h>
#include <vector>
#include <sys/uio.h>

#pragma once

const int int_size = sizeof(int);

inline bool read_process_memory_linux(pid_t pid, uintptr_t address, void* buffer, size_t size) {
    struct iovec local_iov, remote_iov;

    local_iov.iov_base = buffer;
    local_iov.iov_len  = size;

    remote_iov.iov_base = reinterpret_cast<void*>(address);
    remote_iov.iov_len  = size;

    ssize_t nread = process_vm_readv(pid, &local_iov, 1, &remote_iov, 1, 0);
    return nread == (ssize_t)size;
}

inline uintptr_t add_offset32_linux(pid_t pid, uintptr_t base, uintptr_t offset) {
    uintptr_t pointer_address = base + offset;
    uint32_t new_pointer = 0;

    read_process_memory_linux(pid, pointer_address, &new_pointer, sizeof(new_pointer));
    return new_pointer;
}

inline uintptr_t add_offset64_linux(pid_t pid, uintptr_t base, uintptr_t offset) {
    uintptr_t pointer_address = base + offset;
    uint64_t new_pointer = 0;

    read_process_memory_linux(pid, pointer_address, &new_pointer, sizeof(new_pointer));
    return new_pointer;
}

inline uintptr_t add_all_offsets_linux(pid_t pid,uintptr_t base_module,const std::vector<uintptr_t>& offsets,  bool process64)
{
    uintptr_t addr = process64
        ? add_offset64_linux(pid, base_module, offsets[0])
        : add_offset32_linux(pid, base_module, offsets[0]);

    for (size_t i = 1; i < offsets.size() - 1; i++) {
        addr = process64
            ? add_offset64_linux(pid, addr, offsets[i])
            : add_offset32_linux(pid, addr, offsets[i]);
    }

    // last offset is added but NOT dereferenced
    addr += offsets.back();
    return addr;
}

//dereferences once at the end, adds up all offsets directly
inline int read_proc_memory_deref_last(pid_t pid,uintptr_t base_module, const std::vector<uintptr_t>& offsets, bool process64)
{
    uintptr_t final_ptr;

    if (offsets.size() > 1) {
        final_ptr = add_all_offsets_linux(pid, base_module, offsets, process64);
    }
    else {
        final_ptr = base_module + offsets[0];
    }

    int value = 0;
    read_process_memory_linux(pid, final_ptr, &value, sizeof(value));
    return value;
}

inline auto read_proc_memory_c_string(int pid, uintptr_t base_module , std::vector<uintptr_t>offsets, unsigned int offsets_len, char buffer[], size_t buffer_size, bool process64)
{
    uintptr_t last_pointer;
    if (offsets_len >= 1) {
        last_pointer = add_all_offsets_linux(pid, base_module, offsets, process64);
    } else {
        last_pointer = base_module + offsets[0];
    }
    
    read_process_memory_linux(pid, last_pointer, buffer, buffer_size);
    return buffer;
}

//dereferences the base address, adds all offsets then dereferences the result
inline int read_proc_memory_deref_first(pid_t pid,uintptr_t base_module, const std::vector<uintptr_t>& offsets, bool process64) {
    uint32_t class_pointer = 0;
    bool success = read_process_memory_linux(pid, base_module, &class_pointer, int_size);

    if(success) {
        return read_proc_memory_deref_last(pid,class_pointer,offsets,process64);
    }

    return class_pointer;
}

//directly dereferencing the address, no offsets
template <typename T>
T read_direct_deref(pid_t pid, uintptr_t address) {
    T value{};
    read_process_memory_linux(pid, address, &value, sizeof(T));
    return value;
}

inline std::string read_proc_memory_string_unknown_size(pid_t pid, uintptr_t base_module,
    const std::vector<uintptr_t>& offsets, unsigned int offsets_len,
    size_t buffer_size,bool process64
) {
    uintptr_t last_pointer;

    if (offsets_len >= 1 && !offsets.empty()) {
        last_pointer =
            add_all_offsets_linux(pid, base_module, offsets, process64);
    } else {
        last_pointer = base_module;
    }

    std::string buffer;
    buffer.resize(buffer_size);

    while (true) {
        if (!read_process_memory_linux(
                pid,
                last_pointer,
                buffer.data(),
                buffer.size())) {
            return {};
        }

        auto null_pos = buffer.find('\0');
        if (null_pos != std::string::npos) {
            buffer.resize(null_pos);
            return buffer;
        }

        buffer_size *= 2;
        buffer.resize(buffer_size);
    }
}
