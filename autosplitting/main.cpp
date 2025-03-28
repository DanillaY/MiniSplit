#include <iostream>
#include <boost/array.hpp>
#include <boost/asio.hpp>
#include "socket_client.hpp"
#include "memory_reader_win.hpp"

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

int main(int argc, char* argv[])
{
    /*
		after reading a memory address send the split or a stop flag to the socket

    int error = notify_on_split(argc,argv);
    
    if (error != 0){
        std::cout << "Error while creating a socket connection";
        return error;
    }
    */

    int pid = get_process_id_by_name("proc_name.exe");
    uintptr_t base_address = get_base_address(pid); 
	
	if(base_address != 0) {

		uintptr_t offset1 = 0x0001B4D8;
		uintptr_t offset2 = 0x0;
	
		for(;;){
			int err = read_proc_memory(pid, base_address,offset1, offset2);

			if(err < 0) {
				std::cout << ":(";
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(6000));
		}
	}

    return 0;
}