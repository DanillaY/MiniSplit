#include <iostream>
#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <csignal>
#include <iterator>
#include "memory_reader_win.hpp"
#include "basic_proc_info.hpp"
#include "thread_manager.hpp"
#include "basic_pointer_info_minisplit.hpp"

/*
    build with:
    g++ main.cpp -o autosplit -I "path/to/boost"
    -L "path/to/stage/lib"
    -lboost_system-mgw14-mt-s-x64-1_87
    -lboost_filesystem-mgw14-mt-d-x64-1_87
    -lboost_thread-mgw14-mt-s-x64-1_87
    -lws2_32 (only for windows)
    -pthread

    where -I should point to the main boost directory, example "C:/Program Files/boost/boost_1_87_0"
          -L shoult point to the built boost libs, example "C:/Program Files/boost/boost_1_87_0/stage/lib"
          -and flags like lboost_system-mgw14-mt-s-x64-1_87, lboost_thread-mgw14-mt-s-x64-1_87 are the names of .a files in that stage/lib directory
          also if you are building on windows with mingw64 you should use -lws2_32 flag to use windows socket api
    
    after the compilation run .\autosplit game_name.exe localhost
*/

void handle_sigint(int signal) {
    //clean_up_alloc_emory
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
    uintptr_t base_module_address = get_base_address(pid);
    std::shared_ptr<Basic_Process_Info> bpi = std::make_shared<Basic_Process_Info>(process_name,base_module_address,pid);

    std::vector<uintptr_t> offsets1 = {0x038CB78, 0x38, 0x644, 0x4, 0x18, 0x128, 0x8, 0x0};
	std::vector<uintptr_t> offsets2 = {0x38C7F4};
	std::vector<uintptr_t> offsets3 = {0x395A97};
	std::vector<uintptr_t> offsets4 = {0x38BC20, 0x98CC};

    Basic_Pointer_Info_Minisplit<char*> bpoi_start = Basic_Pointer_Info_Minisplit<char*>(offsets1.size(), offsets1, new char[20],"Saving preferences.","",false,false,Signal_split::START);
	Basic_Pointer_Info_Minisplit<int> bpoi_pause = Basic_Pointer_Info_Minisplit<int>(offsets2.size(), offsets2,0, 1,0,true,false,Signal_split::PAUSE);
	Basic_Pointer_Info_Minisplit<char*> bpoi_split_cuts = Basic_Pointer_Info_Minisplit<char*>(offsets3.size(), offsets3, new char[9],"mcvi.bik","",false,false,Signal_split::SPLIT);
	Basic_Pointer_Info_Minisplit<char*> bpoi_split_lev = Basic_Pointer_Info_Minisplit<char*>(offsets4.size(), offsets4, new char[10],"CAKC","BBLT",false,true,Signal_split::SPLIT);

    auto start_memory_reader_start = std::bind(&Thread_Manager::start_memory_reader_string<char*>, &t_manager,bpi, bpoi_start,20, &t_manager);
	auto start_memory_reader_split_cuts = std::bind(&Thread_Manager::start_memory_reader_string<char*>, &t_manager,bpi, bpoi_split_cuts,9, &t_manager);
	auto start_memory_reader_split_lev = std::bind(&Thread_Manager::start_memory_reader_string<char*>, &t_manager,bpi, bpoi_split_lev,10, &t_manager);
	auto start_memory_reader = std::bind(&Thread_Manager::start_memory_reader<int>, &t_manager,bpi, bpoi_pause, &t_manager);
    
    auto start_notifier = std::bind(&Thread_Manager::start_notifier, &t_manager, argc,argv,&t_manager);
    std::vector<std::function<void()>> all_functions = {start_memory_reader_start, start_memory_reader_split_cuts,start_memory_reader_split_lev, start_memory_reader}; //put every reader function here

    auto start_listen_active_process_terminate = std::bind(&Thread_Manager::start_listen_active_process_terminate, &t_manager, bpi, all_functions, true, &t_manager);

    if(pid != 0) {
        
        start_memory_reader_start(bpi, bpoi_start,20, &t_manager);
        start_memory_reader(bpi,bpoi_start, &t_manager);
		start_memory_reader_split_cuts(bpi, bpoi_split_cuts,9, &t_manager);
		start_memory_reader_split_lev(bpi, bpoi_split_lev,10, &t_manager);
        
        start_listen_active_process_terminate(bpi, all_functions, true, &t_manager);
        start_notifier(argc,argv,&t_manager);
    }
    
    return 0;
}