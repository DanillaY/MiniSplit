#include <mutex>
#include <condition_variable>
#include <iostream>
#include <csignal>
#include <vector>
#include <thread>
#include <queue>
#include "memory_reader_win.hpp"
#include "signals_minisplit.hpp"
#include "basic_pointer_info_minisplit.hpp"

#pragma once

class Thread_Manager {

    public:
        std::vector<std::thread> data_readers_threads;
        std::thread notifier_thread;

        std::mutex mutex;
        std::condition_variable condition_var;
        
        std::queue<Signal_split> signal_queue;
        std::atomic<bool> is_process_alive = true;

        Thread_Manager() {
            
        }

        ~Thread_Manager() {

            is_process_alive = false;
            for (auto& t : data_readers_threads) {
                if (t.joinable())
                    t.join();
            }
            
            if (notifier_thread.joinable())
                notifier_thread.join();
            
        }
        
        template <typename T>
        void start_memory_reader_c_string(Basic_Process_Info* bpi, Basic_Pointer_Info<T> bpoi ,unsigned int buffer_size, Thread_Manager* t_manager) {
            data_readers_threads.push_back(std::thread(&Thread_Manager::read_data_c_string<char*>, t_manager, bpi, bpoi, buffer_size));
            data_readers_threads.back().detach();
        }

        template <typename T>
        void start_memory_reader_string(Basic_Process_Info* bpi, Basic_Pointer_Info<T> bpoi ,unsigned int buffer_size, Thread_Manager* t_manager) {
            data_readers_threads.push_back(std::thread(&Thread_Manager::read_data_string_using_sets<std::string>, t_manager, bpi, bpoi, buffer_size));
            data_readers_threads.back().detach();
        }

        template <typename T>
        void start_memory_reader(Basic_Process_Info* bpi, Basic_Pointer_Info<T> bpoi, Thread_Manager* t_manager) {
            data_readers_threads.push_back(std::thread(&Thread_Manager::read_data<T>, t_manager,bpoi, bpi));
            data_readers_threads.back().detach();
        }
    
        void start_notifier(int argc, char* argv[], Thread_Manager* t_manager) {
            std::cout<<"Started notifier" << std::endl;
            notifier_thread= std::thread(&Thread_Manager::notify_on_split, t_manager , argc, argv);
            notifier_thread.join();
        }

        void start_listen_active_process_terminate(Basic_Process_Info* bpi, std::vector<std::function<void ()>> all_functions, bool is_igt ,Thread_Manager* t_manager) {
            std::thread t(&Thread_Manager::listen_active_process_terminate,t_manager, bpi , all_functions, is_igt);
            t.detach();
            
        }

    private:
        template <typename T>
        void read_data_c_string(Basic_Process_Info* bpi, Basic_Pointer_Info<T> bpoi ,size_t buffer_size) {
            
            int pid = bpi->get_pid();
            uintptr_t base_module_address = bpi->get_base_offset();
            
            char previous_value[buffer_size];
            char current_value[buffer_size];

            while(is_process_alive) {

                std::strcpy(previous_value, current_value);
                std::strcpy(current_value, read_proc_memory_c_string(pid, base_module_address, bpoi.offsets, bpoi.offsets_len-1, bpoi.buffer, buffer_size));

                std::this_thread::sleep_for(std::chrono::milliseconds(4));
                bool change_back = bpoi.with_change_back == true ? (strcmp(previous_value, current_value) != 0 && strcmp(current_value, bpoi.compared_to) == 0) || (strcmp(current_value, previous_value) != 0 && strcmp(previous_value, bpoi.compared_to)) == 0: false;
                bool compared_prev = bpoi.with_compare_prev == true ? strcmp(bpoi.compared_to, current_value) == 0 && strcmp(bpoi.compared_to_prev, previous_value) == 0 : false;
                bool current_value_equal_comapred_to = bpoi.with_change_back == true || bpoi.with_compare_prev ==true ? false : strcmp(bpoi.compared_to, current_value) == 0 && strcmp(current_value, previous_value) != 0 && bpoi.sig != Signal_split::NONE;

                if(current_value_equal_comapred_to || compared_prev || change_back) {
                    {
                        std::lock_guard lock(mutex);
                        signal_queue.push(bpoi.sig);
                        condition_var.notify_one();
                    }
                }
            }            
        }

        template <typename T>
        void read_data_string_using_sets(Basic_Process_Info* bpi, Basic_Pointer_Info<T> bpoi ,size_t buffer_size) {
            
            int pid = bpi->get_pid();
            uintptr_t base_module_address = bpi->get_base_offset();
            
            std::string previous_value(buffer_size,'\0');
            std::string current_value(buffer_size,'\0');

            while(is_process_alive) {

                previous_value = current_value;
                current_value = read_proc_memory_string_unknown_size(pid, base_module_address, bpoi.offsets, bpoi.offsets_len-1, bpoi.buffer, buffer_size);
                
                std::this_thread::sleep_for(std::chrono::milliseconds(4));

                bool change_back = bpoi.with_change_back == true ? (previous_value != current_value && bpoi.compared_to_set.count(current_value) > 0) || (current_value != previous_value && bpoi.compared_to_prev_set.count(previous_value) > 0): false;
                bool compared_prev = bpoi.with_compare_prev == true ? bpoi.compared_to_set.count(current_value) > 0 && bpoi.compared_to_prev_set.count(previous_value) > 0 : false;
                bool current_value_equal_comapred_to = bpoi.with_change_back == true || bpoi.with_compare_prev ==true ? false : bpoi.compared_to_set.count(current_value) > 0 && current_value != previous_value && bpoi.sig != Signal_split::NONE;		
                
                if(compared_prev || change_back || current_value_equal_comapred_to) {
                    {
                        std::lock_guard lock(mutex);
                        signal_queue.push(bpoi.sig);
                        condition_var.notify_one();
                    }
                }
            }            
        }

        template <typename T>
        void read_data(Basic_Pointer_Info<T> bpoi, Basic_Process_Info* bpi) {

            int pid = bpi->get_pid();
            uintptr_t base_module_address = bpi->get_base_offset();

            if(base_module_address != 0 && bpoi.offsets_len > 0) {
                int current_value = 0;
                int previous_value = 0;

                while(is_process_alive) {

                    previous_value = current_value;
                    current_value = read_proc_memory(pid, base_module_address, bpoi.offsets, bpoi.offsets_len-1, bpoi.buffer);

                    std::this_thread::sleep_for(std::chrono::milliseconds(4));

                    bool change_back = bpoi.with_change_back == true ? (previous_value != current_value && bpoi.compared_to == current_value) || (current_value != previous_value && bpoi.compared_to != current_value): false;
                    bool compared_prev = bpoi.with_compare_prev == true ? bpoi.compared_to == current_value && bpoi.compared_to_prev == previous_value : false;
                    bool current_value_equal_comapred_to = bpoi.with_change_back == true || bpoi.with_compare_prev ==true ? false : bpoi.compared_to == current_value && current_value != previous_value && bpoi.sig != Signal_split::NONE;		
                    
                    if(compared_prev || change_back || current_value_equal_comapred_to) {
                        {
                            std::lock_guard<std::mutex> lock(mutex);
                            signal_queue.push(bpoi.sig);
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
                    condition_var.wait(lock, [this]() { return signal_queue.empty() == false; });

                    while (signal_queue.empty() == false) {
                        Signal_split sig = signal_queue.front();
                        signal_queue.pop();
                    
                        std::string buffer= To_String(sig);
                        boost::system::error_code error;
                
                        boost::asio::write(socket, boost::asio::buffer(buffer), error);
						
						std::this_thread::sleep_for(std::chrono::milliseconds(2));
                        if(error){
                            std::cerr << "Error while writing data to socket: "<< error.message();
                            return -1;
                        }
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

        void listen_active_process_terminate(Basic_Process_Info* bpi, std::vector<std::function<void()>> all_functions, bool is_igt) {

            char* proc_name = bpi->get_process_name();;
            int pid = get_process_id_by_name(proc_name);

            while(is_process_alive) {
                pid = get_process_id_by_name(proc_name);

                if(pid == 0) {
                    break;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(900));
            }

            {
                std::lock_guard lock(mutex);
                std::cout << bpi->get_process_name() << " process was terminated" << std::endl;
                signal_queue = std::queue<Signal_split>(); //clear all signals
                bpi->set_base_module_address(mutex,0);
                bpi->set_pid(mutex,0);
                is_process_alive = false;
                
                //send pause signal for igt
                if(is_igt) {
                    std::lock_guard<std::mutex> lock(mutex);
                    signal_queue.push(Signal_split::PAUSE);
                    condition_var.notify_one();
                }
                
            }
            
            listen_active_process_alive(bpi,all_functions,proc_name,is_igt);
        }

        void listen_active_process_alive(Basic_Process_Info* bpi, std::vector<std::function<void()>> all_functions, std::string proc_name, bool is_igt) { 

            int pid = 0;

            //start listening for a new process
            while(is_process_alive == false) {

                pid = get_process_id_by_name(proc_name);

                if(pid != 0) {
                    break;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(300));
            }

            {
                std::lock_guard lock(mutex);
                bpi->set_pid(mutex,pid);
                bpi->set_base_module_address(mutex,get_base_address(pid));
                is_process_alive = true;

                //send pause signal again to unpause
                if(is_igt) {
                    signal_queue.push(Signal_split::PAUSE);
                    condition_var.notify_one();
                }
            }
        
            std::cout<< proc_name <<" was found with pid: " << pid << std::endl;
            
            //we start all the memory reading again after the proccess was found
            for (auto& minispli_function : all_functions) {
                minispli_function();
            }

            //start listening if the process is closed again
            listen_active_process_terminate(bpi, all_functions, is_igt);
        }
};