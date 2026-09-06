#include <algorithm>
#include <fstream>
#include <filesystem>
#include <iostream>
#include <cstdint>
#include <string>
#include "memory_region_process.hpp"

#pragma once

/*
    this file contains functions for general purpose use (linux? only)
*/
inline void print_pointer_info(void* pointer, pid_t pid) {
    std::ifstream maps("/proc/" + std::to_string(pid) + "/maps");
    std::string line;
    uintptr_t addr = reinterpret_cast<uintptr_t>(pointer);
    bool readSuccess = false;
    MemoryRegion memRegion;

    while (std::getline(maps, line)) {
        std::istringstream iss(line);
        std::string range, perms, offset, dev, inode, pathname;

        if (!(iss >> range >> perms >> offset >> dev >> inode))
            continue;

        size_t dash = range.find('-');
        uintptr_t start = std::stoul(range.substr(0, dash), nullptr, 16);
        uintptr_t end   = std::stoul(range.substr(dash + 1), nullptr, 16);

        if (addr >= start && addr < end) {
            memRegion.start = start;
            memRegion.end   = end;

            if (iss >> pathname)
                memRegion.pathname = pathname;

            memRegion.perms = perms;
            readSuccess = true;
        }
    }

    if (readSuccess) {
        std::cout << "Base Address: 0x" << std::hex << memRegion.start << std::endl;
        std::cout << "Region Size: 0x" << (memRegion.end - memRegion.start) << std::endl;
        std::cout << "Permissions: " << memRegion.perms << std::endl;
        std::cout << "Mapped File: " << memRegion.pathname << std::endl;
    } 
}

inline int get_process_id_by_name(const std::string& name) {
    namespace fs = std::filesystem;

    for (const auto& entry : fs::directory_iterator("/proc")) {
        if (!entry.is_directory())
            continue;

        const std::string filename = entry.path().filename();
        if (!std::all_of(filename.begin(), filename.end(), ::isdigit))
            continue;

        int pid = std::stoi(filename);

        std::ifstream comm(entry.path() / "comm");
        if (!comm.is_open())
        {
            continue;
        }

        std::string pname;
        std::getline(comm, pname);

        if (pname == name) {
            return pid;
        }
    }

    return -1;
}

inline bool is_64bit_process(pid_t pid) {
    std::string exe_path = "/proc/" + std::to_string(pid) + "/exe";
    char buffer[5] = {0};

    std::ifstream f(exe_path, std::ios::binary);

    if(!f)
    {
        return false;
    }
    f.read(buffer, 5);

    if (!f) 
    {
        return false;
    }

    if (buffer[0] != 0x7f || buffer[1] != 'E' || buffer[2] != 'L' || buffer[3] != 'F') 
    {
        return false;
    }

    if (buffer[4] == 1) return false;
    else if (buffer[4] == 2) return true;
    else return false;
}


inline uintptr_t get_base_address(pid_t pid) {
    std::ifstream maps("/proc/" + std::to_string(pid) + "/maps");
    if (!maps)
        return 0;

    std::string line;
    while (std::getline(maps, line)) {
        std::istringstream iss(line);
        std::string range, perms, offset, dev, inode, pathname;

        if (!(iss >> range >> perms >> offset >> dev >> inode))
            continue;

        uintptr_t start = std::stoul(range.substr(0, range.find('-')), nullptr, 16);

        if (iss >> pathname && pathname.find("/exe") == std::string::npos) {
            continue;
        }

        return start;
    }

    return 0;
}

//for games ran through proton to figure out the correct path this formula should be applied:
//address formula = linux_address = linux_mapping_start + (pince_pointer - linux_pe_base)
uintptr_t get_linux_exe_mapping(pid_t pid) {
    std::ifstream maps("/proc/" + std::to_string(pid) + "/maps");
    if (!maps) return 0;

    std::string line;
    while (std::getline(maps, line)) {
        std::istringstream iss(line);
        std::string range, perms, offset, dev, inode, pathname;

        if (!(iss >> range >> perms >> offset >> dev >> inode))
            continue;

        if (!(iss >> pathname)) continue;
        if (pathname.find(".exe") == std::string::npos) continue;

        size_t dash = range.find('-');
        uintptr_t start = std::stoul(range.substr(0, dash), nullptr, 16);

        return start;
    }

    return 0;
}