#include <iostream>
#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <csignal>
#include <iterator>
#include "memory_reader_win.hpp"
#include "basic_proc_info.hpp"
#include "thread_manager.hpp"

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

    uintptr_t offsets1[] = {0x038CB78, 0x38, 0x644, 0x4, 0x18, 0x128, 0x8, 0x0};
    int offsets_len1 = (sizeof(offsets1)/sizeof(offsets1[0]));

    uintptr_t offsets2[] = {0x38C7F4};
    int offsets_len2 = (sizeof(offsets2)/sizeof(offsets2[0]));

	int buffer = 0;
    char* buffer_str = new char[20]; //buffer is the result of the chain dereference in read_proc_memory
    char* buffer_str2 = new char[20];

	auto start_memory_reader_string = std::bind(&Thread_Manager::start_memory_reader_string, &t_manager,bpi, ".",false,buffer_str,offsets_len1,20,offsets1,Signal_split::START, &t_manager);
	auto start_memory_reader = std::bind(&Thread_Manager::start_memory_reader,&t_manager,bpi,buffer, 1, true, offsets2,offsets_len2,Signal_split::PAUSE, &t_manager);
	auto start_notifier = std::bind(&Thread_Manager::start_notifier, &t_manager, argc,argv,&t_manager);
	
	std::vector<std::function<void()>> all_functions = {start_memory_reader_string,start_memory_reader}; //put every reader function here

	auto start_listen_active_process_terminate = std::bind(&Thread_Manager::start_listen_active_process_terminate, &t_manager, bpi, all_functions, &t_manager);

	if(pid != 0) {
		
		start_memory_reader_string(bpi, "Saving preferences.",false,buffer_str,offsets_len1,20,offsets1,Signal_split::START, &t_manager);
		start_memory_reader(bpi,buffer, 1, true, offsets2,offsets_len2,Signal_split::PAUSE, &t_manager);
		start_listen_active_process_terminate(bpi, all_functions, &t_manager);
		start_notifier(argc,argv,&t_manager);
	}
    
    return 0;
}