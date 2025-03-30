#include <mutex>
#include <condition_variable>
#include <iostream>
#include <csignal>
#include <vector>
#include <thread>
#include "memory_reader_win.hpp"
#include "signals_minisplit.hpp"

#pragma once

class Thread_Manager {

    public:
        std::vector<std::thread> data_readers_threads;
        std::thread notifier_thread;

        std::mutex mutex;
        std::condition_variable condition_var;
        
        Signal_split signal; //used to store a command for minisplit socket
        bool ready = false;

        Thread_Manager() {
            signal = Signal_split::NONE;
        }

        ~Thread_Manager() {

            for (auto& t : data_readers_threads) {
                if (t.joinable())
                    t.join();
            }
            
            if (notifier_thread.joinable())
                notifier_thread.join();
            
        }

        void start_memory_reader_string(Basic_Process_Info bpi, const char* compared_to, bool with_change_back, char* buffer, unsigned int offsets_len,unsigned int char_len, uintptr_t* offsets,Signal_split sig ,Thread_Manager* t_manager) {
            std::thread t(&Thread_Manager::read_data_string, t_manager, compared_to, buffer, offsets_len, offsets,char_len,sig,bpi, with_change_back);
            t.detach();
        }

        void start_memory_reader(Basic_Process_Info bpi, int buffer, int compared_to, bool with_change_back, uintptr_t* offsets, unsigned int offsets_len, Signal_split sig, Thread_Manager* t_manager) {
            std::thread t(&Thread_Manager::read_data, t_manager, buffer,compared_to, bpi.base_module_address, bpi.pid, offsets, offsets_len, sig,with_change_back);
            t.detach();
        }
    
        void start_notifier(int argc, char* argv[], Thread_Manager* t_manager) {
            std::cout<<"Started notifier" << std::endl;
            std::thread t(&Thread_Manager::notify_on_split, t_manager , argc, argv);
            t.join();
        }

    private:

        //with_change_back param is used to send the command twice. once it detects that the current value has changed to the desired one, it sends the signal again after the current value changes, this is useful for tracking things like in-game time, where you can send a PAUSE signal and then unpause
        void read_data_string(const char* compared_to, char* buffer, unsigned int offsets_len, uintptr_t* offsets, unsigned int char_len,Signal_split sig, Basic_Process_Info bpi, bool with_change_back) {
            
            if(bpi.base_module_address != 0 && offsets_len > 0) {

                char previous_value[char_len];
                char current_value[char_len];

                for(;;){
                    std::strcpy(previous_value, current_value);
                    std::strcpy(current_value, read_proc_memory_string(bpi.pid, bpi.base_module_address, offsets, offsets_len-1, char_len, buffer, 20));
                    std::this_thread::sleep_for(std::chrono::milliseconds(3));

                    bool change_back = (strcmp(previous_value, compared_to) != 0 && strcmp(current_value, compared_to) == 0) || (strcmp(current_value, compared_to) != 0 && strcmp(previous_value, compared_to) == 0);
                    
                    if(with_change_back == true ? change_back : (strcmp(compared_to, current_value) == 0 && strcmp(current_value, previous_value) != 0 && sig != Signal_split::NONE)) {
                        {
                            std::lock_guard<std::mutex> lock(mutex);
                            ready = true;
                            signal = sig;
                            condition_var.notify_one();
                        }
                    }

                }
            }
        }

        void read_data(int buffer, int compared_to, uintptr_t base_module_address,int pid, uintptr_t* offsets, unsigned int offsets_len, Signal_split sig, bool with_change_back) {

            if(base_module_address != 0 && offsets_len > 0) {
                int current_value = 0;
                int previous_value = 0;

                for(;;){
                    previous_value = current_value;
                    current_value = read_proc_memory(pid, base_module_address, offsets, offsets_len-1, buffer);
                    std::this_thread::sleep_for(std::chrono::milliseconds(5));

                    bool change_back = (current_value == compared_to && current_value != previous_value) || (previous_value = compared_to && current_value != previous_value);

                    if(with_change_back == true ? change_back : (compared_to == current_value && current_value != previous_value && sig != Signal_split::NONE)) {
                        {
                            std::lock_guard<std::mutex> lock(mutex);
                            ready = true;
                            signal = sig;
                            condition_var.notify_one();
                        }
                    }
                    
                }
            }
        }

        int notify_on_split(int argc, char* argv[]) {

            try
            {
                using boost::asio::ip::tcp;

                boost::asio::io_context io_context;
        
                tcp::resolver resolver(io_context);
                tcp::resolver::results_type endpoints = resolver.resolve(argv[2], "5554");
        
                tcp::socket socket(io_context);
                boost::asio::connect(socket, endpoints);
                
                for(;;) {
                    std::unique_lock<std::mutex> lock(mutex);
                    condition_var.wait(lock, [this]() { return ready; });
            
                    std::string buffer= To_String(signal);
                    boost::system::error_code error;
            
                    boost::asio::write(socket, boost::asio::buffer(buffer), error);
            
                    //return values to its original state
                    signal = Signal_split::NONE;
                    ready = false;

                    std::this_thread::sleep_for(std::chrono::milliseconds(2));
                    if(error){
                        std::cerr << "Error while writing data to socket: "<< error.message();
                        return -1;
                    }
                }
            }
            catch (std::exception& e)
            {
                std::cerr << e.what() << std::endl;
                return -1;
            }
        
            return 0;
        }
};