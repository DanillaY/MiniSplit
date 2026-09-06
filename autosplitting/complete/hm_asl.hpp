#include <cstdint>
#include <iostream>
#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <ostream>
#include <string>
#include <sys/types.h>
#include "../memory_reader_base_linux.hpp"
#include "../signature_scanner_linux.hpp"
#include "../basic_proc_info.hpp"
#include "../memory_hook_linux.hpp"
#include "../thread_manager.hpp"
#include <inttypes.h>
#include <stdint.h>

inline void boot_hm_asl(int argc, char* argv[]) {

    Thread_Manager t_manager;
    char* process_name = argv[1];
    int pid = get_process_id_by_name(process_name);
    
    if(pid == -1 || pid == 0) {
        std::cout << "Could not get pid value" << std::endl;
        return;
    }
    
    uintptr_t base_module_address = get_base_address(pid);
    bool is_64bit = is_64bit_process(pid);

    std::vector<uintptr_t> offsets1 = {0x3bc, 0x64c, 0x800, 0x7dc, 0x7d0};
    std::vector<uintptr_t> offsets2 = {};

    Basic_Process_Info& bpi = Basic_Process_Info::get_instance(process_name,base_module_address,pid,is_64bit);
    
    Basic_Pointer_Info<std::int32_t> bpoi_start = Basic_Pointer_Info<std::int32_t>(
        offsets1.size(), 
        offsets1,
        std::int32_t{0},
        std::int32_t{0},
        std::int32_t{144834545},
        false,
        true,
        Signal_split::START);

    Basic_Pointer_Info<std::int32_t> bpoi_pause = Basic_Pointer_Info<std::int32_t>(
        offsets2.size(),
        offsets2,
        std::int32_t{0},
        std::int32_t{19296},
        std::int32_t{144},
        false,
        true,
        Signal_split::PAUSE);

    auto reader_start = [](int pid, uintptr_t base, const std::vector<uintptr_t>& offsets, bool is64) -> std::int32_t {
        std::int32_t start = 0;
        read_process_memory_linux(pid, 0x9ad12b4, &start, sizeof(std::int32_t));

        return start;
    };

    auto reader_pause = [](int pid, uintptr_t base, const std::vector<uintptr_t> offsets, bool is64) -> std::int32_t {
        std::int32_t pause = 0;
        read_process_memory_linux(pid, 0x2626080, &pause, sizeof(std::int32_t));

        return pause;
    };

    auto start_memory_reader_pause = [&]() {
        t_manager.start_memory_reader<std::int32_t>(&bpi, bpoi_pause, &t_manager, reader_pause, 0x400000);
    };

    auto start_memory_reader_start = [&]() {
        t_manager.start_memory_reader<std::int32_t>(&bpi, bpoi_start, &t_manager, reader_start, 0x400000);
    };

    auto start_notifier = [&]() {
        t_manager.start_notifier(argc, argv, &t_manager);
    };
    
    std::vector<std::function<void()>> all_functions = { 
        start_memory_reader_pause,
        start_memory_reader_start
    };

    auto start_listen_active_process_terminate = std::bind(
        &Thread_Manager::start_listen_active_process_terminate, 
        &t_manager, &bpi,
        all_functions, true,
        &t_manager);
        
    start_memory_reader_pause();
    start_memory_reader_start();
    
    start_listen_active_process_terminate();
    start_notifier();
}