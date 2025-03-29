#include <iostream>
#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <csignal>
#include <iterator>
#include "memory_reader_win.hpp"
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
    
    after the compilation run .\autosplit localhost
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

	uintptr_t offsets1[] = {0x038CB78, 0x38, 0x644, 0x4, 0x18, 0x128, 0x8, 0x0};
	int offsets_len1 = (sizeof(offsets1)/sizeof(offsets1[0]));
	Signal_split sig = START;
    
	t_manager.start_memory_reader_string(process_name,offsets1, "string to compare" ,base_module_address,offsets_len1,pid, sig , &t_manager);
	t_manager.start_notifier(argc,argv,&t_manager);

    return 0;
}