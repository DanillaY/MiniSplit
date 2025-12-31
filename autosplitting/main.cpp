#include <cstddef>
#include <cstdint>
#include <iostream>
#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <csignal>
#include <ostream>
#include <string>
#include <sys/types.h>
#include <unordered_set>
//#include "memory_reader_win.hpp"
#include "memory_reader_base_linux.hpp"
#include "memory_reader_linux.hpp"
#include "basic_proc_info.hpp"
#include "memory_hook_linux.hpp"
#include "thread_manager.hpp"
#include "basic_pointer_info_minisplit.hpp"

/*
    on windows build with:
    g++ main.cpp -o autosplit -I "path/to/boost"
    -L "path/to/stage/lib"
    -lboost_system-mgw14-mt-s-x64-1_87 (some libs like system are now header only so could be outdated)
    -lboost_filesystem-mgw14-mt-d-x64-1_87
    -lboost_thread-mgw14-mt-s-x64-1_87
    -lws2_32 (only for windows)
    -pthread

    where -I should point to the main boost directory, example "C:/Program Files/boost/boost_1_87_0"
          -L shoult point to the built boost libs, example "C:/Program Files/boost/boost_1_87_0/stage/lib"
          -and flags like lboost_system-mgw14-mt-s-x64-1_87, lboost_thread-mgw14-mt-s-x64-1_87 are the names of .a files in that stage/lib directory
          also if you are building on windows with mingw64 you should use -lws2_32 flag to use windows socket api
    
    on linux build with:
        g++ main.cpp -o autosplit 
        -lboost_filesystem 
        -lboost_thread 
        -pthread
    
    where lboost_thread and lboost_filesystem are links to boost libs

    after the compilation run .\autosplit game_name.exe localhost
*/

Basic_Process_Info* Basic_Process_Info::bpi = nullptr;

void handle_sigint(int signal) {
    //clean_up_alloc_memory
    exit(0);
}

int main(int argc, char* argv[])
{
	signal(SIGINT, handle_sigint);

    if(argc < 3) {
        std::cout << "Not enough arguments were passed.\nProgram requires a process name argument and the ip of the socket server" << std::endl;
        return 1;
    }

    Thread_Manager t_manager;
    char* process_name = argv[1];
    int pid = get_process_id_by_name(process_name);
    
    if(pid == -1 || pid == 0) {
        std::cout << "Could not get pid value" << std::endl;
        return 1;
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
        return read_direct_deref<int>(pid, 0x26D2A40);
    };

    auto reader_start = [](int pid, uintptr_t base, const std::vector<uintptr_t>& offsets, bool is64) -> int {
        return read_proc_memory_deref_first(pid, 0x0078BC20, offsets, is64);
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

    if(pid != 0) {
        
        start_memory_reader_start();
        start_memory_reader_pause();
		start_memory_reader_split_cuts();
		start_memory_reader_split_lev();
        
        start_listen_active_process_terminate();
        start_notifier();
    }   
}