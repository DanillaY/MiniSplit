#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <ios>
#include <iostream>
#include <string>
#include <fstream> 
#include <sys/types.h>
#include <vector>
#include <sstream>
#include <sys/uio.h>
#include <sys/ptrace.h>
#include <unistd.h>
#include <cstring>
#include <vector>
#include <cerrno>
#include <iostream>


// signature scanner usage example
// Signature sig = Signature("48 8B ?? ?? ?? ?? ?? 25 F0 3F 00 00");
// auto point = scan_for_pattern(sig, pid, 0x7);
// if(point == 0) {
//     std::cout << ":(";
// } else {
//     printf("Value in hex: %" PRIxPTR "\n", point);
// }

class Signature {
    std::vector<uint> bytes;
    std::vector<bool> mask;

    bool matches(unsigned char* ptr) const {

        for (size_t i = 0; i < bytes.size(); ++i) {
            if (mask[i] && ptr[i] != bytes[i]) {
                return false;
            }
        }

        return true;
    }

    public:
    Signature(const std::string& pattern) {
        
        std::istringstream iss(pattern);
        std::string token;

        while (iss >> token) {
            std::transform(token.begin(), token.end(), token.begin(), ::tolower);

            if (token == "??" || token == "*") {
                bytes.push_back(0x00);
                mask.push_back(false);
            } else {
                unsigned int val = 0;

                if (token.substr(0, 2) == "0x") {
                    token = token.substr(2);
                }
                val = std::stoul(token, nullptr, 16);

                bytes.push_back(static_cast<unsigned char>(val));
                mask.push_back(true);
            }
        }
    }

    void* scan_memory(unsigned char* start, unsigned char* end) {

        auto pattern_size = bytes.size();

        if((end-start) < bytes.size()) {
            return nullptr;
        }

        unsigned char* limit = end - pattern_size + 1;
        for(unsigned char* p = start;p< limit;++p) {
            if(matches(p)) {
                return reinterpret_cast<unsigned char*>(p);
            }
        }

        return nullptr;
    }

    std::vector<unsigned char> read_remote_memory(pid_t pid, void* addr, size_t size) {
        if (size == 0) {
            return std::vector<unsigned char>{};
        }

        std::vector<unsigned char> buffer(size);
        struct iovec local = { buffer.data(), buffer.size() };
        struct iovec remote = { addr, size };

        ssize_t nread = process_vm_readv(pid, &local, 1, &remote, 1, 0);

        if (nread == -1) {
            std::cout << "Process_vm_readv failed at " << addr << " (size=" << size << "): " << strerror(errno) << std::endl;
            return std::vector<unsigned char>{};
        }

        if (static_cast<size_t>(nread) != size) {
            buffer.resize(nread);
        }

        return buffer;
    }
};

inline std::vector<std::pair<unsigned char*, unsigned char*>> get_mappings(pid_t pid) {
    std::vector<std::pair<unsigned char*, unsigned char*>> mappings;
    std::string maps_path = "/proc/" + std::to_string(pid) + "/maps";
    std::ifstream maps_file(maps_path);
    std::string line;

    if (!maps_file.is_open()) {
        std::cerr << "Failed to open " << maps_path << std::endl;
        return mappings;
    }

    while (std::getline(maps_file, line)) {
        size_t dash_pos = line.find('-');
        size_t space_pos = line.find(' ', dash_pos);

        if (dash_pos == std::string::npos || space_pos == std::string::npos) {
            continue;
        }

        std::string start_str = line.substr(0, dash_pos);
        std::string end_str = line.substr(dash_pos + 1, space_pos - dash_pos - 1);

        unsigned long long start_ull = 0, end_ull = 0;
        try {
            start_ull = std::stoull(start_str, nullptr, 16);
            end_ull = std::stoull(end_str, nullptr, 16);
        } catch (...) {
            std::cout << "Failed to parse hex addresses in line: " << line << std::endl;
            continue;
        }

        if (start_ull >= end_ull) {
            continue;
        }

        std::istringstream iss(line.substr(space_pos + 1));
        std::string perms, rest;

        if (!(iss >> perms)) {
            continue;
        }

        size_t path_start = line.find('/', space_pos);
        std::string path;

        if (path_start != std::string::npos) {
            path = line.substr(path_start);
            path.erase(path.find_last_not_of(" \t") + 1);

        } else {
            size_t bracket_start = line.find('[', space_pos);
            if (bracket_start != std::string::npos) {
                size_t bracket_end = line.find(']', bracket_start);
                if (bracket_end != std::string::npos) {
                    path = line.substr(bracket_start, bracket_end - bracket_start + 1);
                }
            }
        }

        bool is_exec = (perms.size() >= 3 && perms[2] == 'x');
        bool is_readable = (perms.size() >= 1 && perms[0] == 'r');
        bool is_anon = (path.empty() || path == "[anon]");

        if (path == "[vdso]"  || path == "[vsyscall]" || path == "[vvar_vclock]" || path.find("/dev/") == 0) {
            continue;
        }

        if (is_exec || is_readable) {
            auto start_ptr = reinterpret_cast<unsigned char*>(static_cast<uintptr_t>(start_ull));
            auto end_ptr = reinterpret_cast<unsigned char*>(static_cast<uintptr_t>(end_ull));

            mappings.push_back({start_ptr, end_ptr});

            std::cout << "Accepted: " << std::hex
                      << reinterpret_cast<void*>(start_ptr) << "-"
                      << reinterpret_cast<void*>(end_ptr)
                      << " " << perms << " " << (path.empty() ? "[anon]" : path)
                      << std::dec << std::endl;
        }
    }

    return mappings;
}

//padding - used as offset bytes, it adds this offset after finding the signature. skip_amount - used to specify the exact signature (in case if there are more then 1 entry)
inline uintptr_t scan_for_pattern(Signature signature, pid_t pid, uint padding = 0x0, uint skip_amount = 0) {
    std::cout << "Scanning for pattern in PID: " << pid << std::endl;
    auto mappings = get_mappings(pid);
    uint found_sig_counter = 0;

    if (mappings.empty()) {
        std::cerr << "No memory mappings found for PID " << pid << std::endl;
        return 0;
    }

    for (auto& [start, end] : mappings) {
        size_t size = static_cast<size_t>(end - start);
        if (size == 0) {
            continue; 
        }

        std::cout << "Scanning region: " 
                  << reinterpret_cast<void*>(start) << " - " 
                  << reinterpret_cast<void*>(end) << " (" << size << " bytes)" << std::endl;

        auto mem = signature.read_remote_memory(pid, start, size);

        if (mem.empty()) {
            //skip unreadable mapping
            continue;
        }

        unsigned char* local_start = mem.data();
        unsigned char* local_end   = mem.data() + mem.size();

        void* found = signature.scan_memory(local_start, local_end);

        if (found) {
            found_sig_counter += 1;
            
            if (found_sig_counter >= skip_amount) {
                uintptr_t offset = reinterpret_cast<uintptr_t>(found) - reinterpret_cast<uintptr_t>(local_start);
                return reinterpret_cast<uintptr_t>(start) + offset + padding;
            }
        }
    }

    return 0;
}