//helper struct for getting general process data on linux
#include <cstdint>
#include <string>

#pragma once

struct MemoryRegion {
    uintptr_t start;
    uintptr_t end;
    std::string perms;
    std::string pathname;
};