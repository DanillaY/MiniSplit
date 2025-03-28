#include <iostream>
#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <iterator>
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

    if(argc < 2) {
        std::cout << "Not enough arguments were passed.\nProgram requires a process name argument and the ip of the socket server" << std::endl;
        return 1;
    }
    
    int pid = get_process_id_by_name(argv[1]); //getting the second argument because the first arg is the path to the executable
    uintptr_t base_module_address = get_base_address(pid); 
    uintptr_t offsets1[] = { 0x038CB78, 0x38, 0x644, 0x4, 0x18, 0x128, 0x8, 0x0 };
    uintptr_t offsets2[] = { 0x038CB88, 0x10, 0x84, 0x30, 0x10, 0x8, 0x0};
    uintptr_t offsets3[] = { 0x038CB88, 0x10, 0x84, 0x10, 0x10, 0x8, 0x0};
    
    if(base_module_address != 0 && (std::size(offsets1)-1) > 0) {

        char* buffer = new char[22]; //buffer is the result of the chain dereference in read_proc_memory

        for(;;){
            read_proc_memory_string(pid, base_module_address, offsets1, std::size(offsets1)-1, buffer);
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }

    return 0;
}