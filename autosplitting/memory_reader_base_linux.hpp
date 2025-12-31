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

uintptr_t get_windows_pe_base(pid_t pid, uintptr_t linux_mapping_start) {
    std::ifstream mem("/proc/" + std::to_string(pid) + "/mem", std::ios::binary);
    if (!mem) return 0;

    mem.seekg(linux_mapping_start);

    struct DOSHeader { uint16_t e_magic; uint16_t e_cblp; uint16_t e_cp; 
                       uint16_t e_crlc; uint16_t e_cparhdr; uint16_t e_minalloc;
                       uint16_t e_maxalloc; uint16_t e_ss; uint16_t e_sp;
                       uint16_t e_csum; uint16_t e_ip; uint16_t e_cs;
                       uint16_t e_lfarlc; uint16_t e_ovno; uint16_t e_res[4];
                       uint16_t e_oemid; uint16_t e_oeminfo; uint16_t e_res2[10];
                       int32_t  e_lfanew; };
    DOSHeader dos;
    mem.read(reinterpret_cast<char*>(&dos), sizeof(dos));

    if (dos.e_magic != 0x5A4D) return 0;
    mem.seekg(linux_mapping_start + dos.e_lfanew);

    // Read PE signature and Optional Header magic
    uint32_t pe_sig;
    mem.read(reinterpret_cast<char*>(&pe_sig), sizeof(pe_sig));
    if (pe_sig != 0x00004550) return 0;

    uint16_t machine;
    uint16_t number_of_sections;
    mem.read(reinterpret_cast<char*>(&machine), sizeof(machine));
    mem.read(reinterpret_cast<char*>(&number_of_sections), sizeof(number_of_sections));

    mem.seekg(16, std::ios::cur);

    uint16_t magic;
    mem.read(reinterpret_cast<char*>(&magic), sizeof(magic));

    uintptr_t image_base;
    if (magic == 0x10B) {
        uint32_t ib;
        mem.seekg(28, std::ios::cur);
        mem.read(reinterpret_cast<char*>(&ib), sizeof(ib));
        image_base = ib;
    } else if (magic == 0x20B) {
        uint64_t ib;
        mem.seekg(24, std::ios::cur);
        mem.read(reinterpret_cast<char*>(&ib), sizeof(ib));
        image_base = ib;
    }

    return image_base;
}
