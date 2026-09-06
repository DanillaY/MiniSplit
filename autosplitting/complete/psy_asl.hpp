#include <cstddef>
#include <cstdint>
#include <iostream>
#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <ostream>
#include <string>
#include <sys/types.h>
#include <unordered_set>
#include "../memory_reader_base_linux.hpp"
#include "../memory_reader_linux.hpp"
#include "../signature_scanner_linux.hpp"
#include "../basic_proc_info.hpp"
#include "../memory_hook_linux.hpp"
#include "../thread_manager.hpp"
#include "../basic_pointer_info_minisplit.hpp"
#include <inttypes.h>
#include <stdint.h>

inline void boot_psy_asl(int argc, char* argv[]) {
    Thread_Manager t_manager;
    char* process_name = argv[1];
    int pid = get_process_id_by_name(process_name);
    
    if(pid == -1 || pid == 0) {
        std::cout << "Could not get pid value" << std::endl;
        return;
    }
    uintptr_t base_module_address = get_base_address(pid);
    bool is_64bit = is_64bit_process(pid);

    Basic_Process_Info& bpi = Basic_Process_Info::get_instance(process_name,base_module_address,pid,is_64bit);

    std::vector<uintptr_t> offsets1 = {0x89F5};
    std::vector<uintptr_t> offsets2 = {0x98FD};
    std::vector<uintptr_t> offsets3 = {};
    std::vector<uintptr_t> offsets4 = {};
    std::unordered_set<std::string> curr_values_split = {"cakc.plb","casa.plb", "cabh.plb", "casa.plb", "cabh_night.plb", "loma.plb","tcama_night.plb", "asgr.plb","asgr.plb","asgr.plb","asco.plb","asco.plb","mctc.plb"};
    std::unordered_set<std::string> prev_values_split = {"bblt.plb","sacu.plb", "mill.plb", "", "cagp_night.plb", "llll.plb", "cali_night.plb", "locb.plb","mmdm.plb", "thfb.plb","bvma.plb","wwma.plb","asru.plb"};
    std::unordered_set<std::string> prev_value_cuts = {".pba"};
    std::unordered_set<std::string> curr_value_cuts = {"mcvi.bik"};
 
    Basic_Pointer_Info<std::byte> bpoi_start = Basic_Pointer_Info<std::byte>(offsets1.size(), offsets1, std::byte{0},std::byte{01},std::byte{0},false,false,Signal_split::START);
	Basic_Pointer_Info<std::byte> bpoi_pause = Basic_Pointer_Info<std::byte>(offsets2.size(), offsets2,std::byte{0}, std::byte{01},std::byte{0},true,false,Signal_split::PAUSE);
	Basic_Pointer_Info<std::string> bpoi_split_cuts = Basic_Pointer_Info<std::string>(offsets3.size(), offsets3, std::string(),curr_value_cuts,prev_value_cuts,false,false,Signal_split::SPLIT);
	Basic_Pointer_Info<std::string> bpoi_split_lev = Basic_Pointer_Info<std::string>(offsets4.size(), offsets4, std::string(),curr_values_split,prev_values_split,false,true,Signal_split::SPLIT);

    auto reader_pause = [](int pid, uintptr_t base, const std::vector<uintptr_t>& offsets, bool is64) -> int {
        auto pause_flag = 0;
        read_process_memory_linux(pid, 0xB0E5B7, &pause_flag, sizeof(std::byte));
        return pause_flag;
    };

    auto reader_start = [](int pid, uintptr_t base, const std::vector<uintptr_t>& offsets, bool is64) -> int {
        return read_proc_memory_deref_first(pid, 0x78BC20, offsets, is64);
    };

    auto start_memory_reader_start = [&]() {
        t_manager.start_memory_reader<std::byte>(&bpi, bpoi_start, &t_manager, reader_start);
    };

    auto start_memory_reader_split_lev = [&]() {
        t_manager.start_memory_reader_string<std::string>(&bpi, bpoi_split_lev, 4, &t_manager, 0x026D2A55, true);
    };

    auto start_memory_reader_split_cuts = [&]() {
        t_manager.start_memory_reader_string<std::string>(&bpi, bpoi_split_cuts, 9, &t_manager, 0x795A97, true);
    };

    auto start_memory_reader_pause = [&]() {
        t_manager.start_memory_reader<std::byte>(&bpi, bpoi_pause, &t_manager, reader_pause);
    };

    auto start_notifier = [&]() {
        t_manager.start_notifier(argc, argv, &t_manager);
    };
    
    std::vector<std::function<void()>> all_functions = { 
        start_memory_reader_start,
        start_memory_reader_pause,
        start_memory_reader_split_lev,
        start_memory_reader_split_cuts,
    }; //put every reader function here

    auto start_listen_active_process_terminate = std::bind(
        &Thread_Manager::start_listen_active_process_terminate, 
        &t_manager, &bpi,
        all_functions, true,
        &t_manager);
        
    start_memory_reader_start();
    start_memory_reader_pause();
    start_memory_reader_split_cuts();
    start_memory_reader_split_lev();
    
    start_listen_active_process_terminate();
    start_notifier();
}