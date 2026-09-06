#include <iostream>
#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <csignal>
#include <ostream>
#include <sys/types.h>
#include <inttypes.h>
#include <stdint.h>
#include "./complete/hm_asl.hpp"


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
         -O3 -march=native -mtune=native
         -flto -pthread   
         -lboost_filesystem -lboost_thread
    
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

    boot_hm_asl(argc, argv);
}