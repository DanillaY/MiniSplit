//useful for debugging purposes
#include <Windows.h>
#include <iostream>
#include <psapi.h>
#include <string>

/*
    this file contains functions for general purpose use (windows only)
*/
void print_pointer_info(HANDLE handle, void* pointer) {
    MEMORY_BASIC_INFORMATION mbi;

    if (VirtualQueryEx(handle, pointer, &mbi, sizeof(mbi))) {
        std::cout << "Base Address: " << std::hex << mbi.BaseAddress << std::endl;
        std::cout << "Allocation Base: " << std::hex << mbi.AllocationBase << std::endl;
        std::cout << "Region Size: " << std::hex << mbi.RegionSize << std::endl;
        std::cout << "State: " << mbi.State << std::endl;
        std::cout << "Protect: " << std::hex << mbi.Protect << std::endl;
        std::cout << "Type: " << mbi.Type << std::endl;
    } 
}

bool is_64bit_process(HANDLE handle) {
    BOOL isWow64 = FALSE;

    if (IsWow64Process(handle, &isWow64)) {
        return !isWow64;
    }

    return false;
}
 
int get_process_id_by_name(std::string proc_name){
    DWORD proc_arr[1024] = {};
    DWORD cb_needed = 0;
    DWORD process_count = 0;

    if (EnumProcesses(proc_arr, sizeof(proc_arr), &cb_needed) == false) {
        std::cerr << "Failed process init" << std::endl;
        return -1;
    }

    process_count  = cb_needed / sizeof(DWORD);
    
    for(int i =0 ;i< process_count; i++) {

        DWORD pid = proc_arr[i];
        char runnig_proc_name[256] = "unknown";
        HANDLE handler_proc = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);

        if(GetModuleBaseNameA(handler_proc, NULL, runnig_proc_name, sizeof(runnig_proc_name) / sizeof(char)) > 0){
            if(runnig_proc_name == proc_name){
                CloseHandle(handler_proc);
                return pid;
            }
        }

        CloseHandle(handler_proc);
    }

    std::cout << "Process with the set name was not found" << std::endl;
    return 0;
}

uintptr_t get_base_address(int pid) {

    HANDLE handler = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
    HMODULE main_module;
    DWORD module_buffer;
    TCHAR file_name[MAX_PATH];

    if(EnumProcessModules(handler,&main_module,sizeof(main_module), &module_buffer)) {

        if (GetModuleFileNameEx(handler, main_module, file_name, sizeof(file_name) / sizeof(TCHAR)) && main_module != nullptr) {
            std::wcout << L"Base address of main module: " << std::hex << main_module << std::endl;
            std::wcout << L"Module file: " << file_name << std::endl;

            return (uintptr_t)main_module;
        }

        CloseHandle(handler);
        return 0;
    }
    else {
        std::cout << "Could not get the base module address" << std::endl;
        CloseHandle(handler);
        return 0;
    }
}