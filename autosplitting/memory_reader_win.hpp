#include <Windows.h>
#include <iostream>
#include <psapi.h>
#include <string>
#include "memory_reader_base_win.hpp"
#include <cstdint>

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
    uint32_t new_pointer1;
    ReadProcessMemory(handle, (LPCVOID)pointer1_address, &new_pointer1, sizeof(new_pointer1), nullptr);

	return new_pointer1;
}

int read_proc_memory(int pid, uintptr_t base_module, uintptr_t offset1, uintptr_t offset2) {

    HANDLE handle = OpenProcess(PROCESS_VM_READ | PROCESS_VM_OPERATION |PROCESS_QUERY_INFORMATION, FALSE, pid);
	bool process64 = is_64bit_process(handle);

    bool buffer = false;
    print_pointer_info(handle, (void*)base_module);

	uintptr_t base_and_offset1 =  process64 ? add_offset64(handle,base_module,offset1): add_offset32(handle,base_module,offset1);
	uintptr_t pointer2_address = base_and_offset1 + offset2;

    //ReadProcessMemory(handle, (LPCVOID)pointer2_address, &buffer, sizeof(buffer), nullptr);
	//std::cout << "Value from a pointer: " << buffer << std::endl;
    
    if (ReadProcessMemory(handle, LPCVOID(pointer2_address), &buffer, sizeof(buffer), nullptr)) {
        std::cout << "Value from a pointer: " << buffer << std::endl;

    } else {
        DWORD error = GetLastError();
        std::cerr << "read_proc_memory failed. Error code: " << error << std::endl;
        return -1;
    }

    return 0;
}
