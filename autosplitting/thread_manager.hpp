#include <mutex>
#include <condition_variable>
#include <iostream>
#include <csignal>
#include <vector>
#include <thread>
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
        
        Signal_split signal; //used to store a command for minisplit socket
        bool ready = false;
        std::atomic<bool> is_process_alive = true;

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
		
		//even tho i set read_data_string to char* type i still need a generic type for basic pointer info
		template <typename T>
        void start_memory_reader_string(std::shared_ptr<Basic_Process_Info> bpi, Basic_Pointer_Info_Minisplit<T> bpoi ,unsigned int char_len, Thread_Manager* t_manager) {
            data_readers_threads.push_back(std::thread(&Thread_Manager::read_data_string<char*>, t_manager, bpi, bpoi, char_len));
            data_readers_threads.back().detach();
            
        }

		template <typename T>
        void start_memory_reader(std::shared_ptr<Basic_Process_Info> bpi, Basic_Pointer_Info_Minisplit<T> bpoi, Thread_Manager* t_manager) {
            {
                std::lock_guard lock(mutex);
                data_readers_threads.push_back(std::thread(&Thread_Manager::read_data<T>, t_manager,bpoi, bpi->get_base_module_address(mutex), bpi->get_pid(mutex)));
                data_readers_threads.back().detach();
            }
        }
    
        void start_notifier(int argc, char* argv[], Thread_Manager* t_manager) {
            std::cout<<"Started notifier" << std::endl;
            notifier_thread= std::thread(&Thread_Manager::notify_on_split, t_manager , argc, argv);
            notifier_thread.join();
        }

        void start_listen_active_process_terminate(std::shared_ptr<Basic_Process_Info> bpi, std::vector<std::function<void ()>> all_functions, bool is_igt ,Thread_Manager* t_manager) {
            std::thread t(&Thread_Manager::listen_active_process_terminate,t_manager, bpi , all_functions, is_igt);
            t.detach();
            
        }

    private:

        template <typename T>
		void read_data_string(std::shared_ptr<Basic_Process_Info> bpi, Basic_Pointer_Info_Minisplit<T> bpoi ,unsigned int char_len) {
            
            int pid = 0;
            uintptr_t base_module_address = 0;
            {
                //check if the base module and an array of offsets is valid
                std::lock_guard lock(mutex);
                if(bpi->get_base_module_address(mutex) == 0 || bpoi.offsets_len <= 0) {
                    return;
                } else {
                    pid = bpi->get_pid(mutex);
                    base_module_address = bpi->get_base_module_address(mutex);
                }
            }
            
            char previous_value[char_len];
            char current_value[char_len];

            while(is_process_alive) {

                std::strcpy(previous_value, current_value);
                std::strcpy(current_value, read_proc_memory_string(pid, base_module_address, bpoi.offsets, bpoi.offsets_len-1, bpoi.buffer, char_len));

                std::this_thread::sleep_for(std::chrono::milliseconds(5));

                bool change_back = bpoi.with_change_back == true ? (strcmp(previous_value, current_value) != 0 && strcmp(current_value, bpoi.compared_to) == 0) || (strcmp(current_value, previous_value) != 0 && strcmp(previous_value, bpoi.compared_to) == 0): false;
				bool compared_prev = bpoi.with_compare_prev == true ? strcmp(bpoi.compared_to, current_value) == 0 && strcmp(bpoi.compared_to_prev, previous_value) == 0 : false;
				bool current_value_equal_comapred_to = bpoi.with_change_back == true || bpoi.with_compare_prev ==true ? false : strcmp(bpoi.compared_to, current_value) == 0 && strcmp(current_value, previous_value) != 0 && bpoi.sig != Signal_split::NONE;

                if(compared_prev || change_back || current_value_equal_comapred_to) {
                    {
                        std::lock_guard lock(mutex);
                        ready = true;
                        signal = bpoi.sig;
                        condition_var.notify_one();
                    }
                }
            }
            
        }

		template <typename T>
        void read_data(Basic_Pointer_Info_Minisplit<T> bpoi, uintptr_t base_module_address,int pid) {

            if(base_module_address != 0 && bpoi.offsets_len > 0) {
                int current_value = 0;
                int previous_value = 0;

                while(is_process_alive) {
                    previous_value = current_value;
                    current_value = read_proc_memory(pid, base_module_address, bpoi.offsets, bpoi.offsets_len-1, bpoi.buffer);
                    std::this_thread::sleep_for(std::chrono::milliseconds(5));

                    bool change_back = (current_value == bpoi.compared_to && current_value != previous_value) || (previous_value = bpoi.compared_to && current_value != previous_value);
					bool current_value_equal_comapred_to = bpoi.compared_to == current_value && current_value != previous_value && bpoi.sig != Signal_split::NONE;

                    if(bpoi.with_change_back == true ? change_back : current_value_equal_comapred_to) {
                        {
                            std::lock_guard<std::mutex> lock(mutex);
                            ready = true;
                            signal = bpoi.sig;
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

        void listen_active_process_terminate(std::shared_ptr<Basic_Process_Info> bpi, std::vector<std::function<void()>> all_functions, bool is_igt) {

            std::string proc_name = "";
            int pid=0;
            {
                std::lock_guard lock(mutex);
                proc_name = bpi->get_process_name(mutex);
                pid = get_process_id_by_name(proc_name);
            }

            while(is_process_alive) {
                pid = get_process_id_by_name(proc_name);

                if(pid == 0) {
                    break;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(900));
            }

            {
                std::lock_guard lock(mutex);
                std::cout << bpi->get_process_name(mutex) << " process was terminated" << std::endl;
                ready = false;
                bpi->set_base_module_address(mutex,0);
                bpi->set_pid(mutex,0);
                is_process_alive = false;
				
				//send pause signal for igt
				if(is_igt) {
					ready = true;
					signal = Signal_split::PAUSE;
					condition_var.notify_one();
				}
				
            }
            
            listen_active_process_alive(bpi,all_functions,proc_name,is_igt);
        }

        void listen_active_process_alive(std::shared_ptr<Basic_Process_Info> bpi, std::vector<std::function<void()>> all_functions, std::string proc_name, bool is_igt) { 

			int pid = 0;

            //start listening for a new process
            while(is_process_alive == false) {

                pid = get_process_id_by_name(proc_name);

                if(pid != 0) {
                    break;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }

			{
				std::lock_guard lock(mutex);
				bpi->set_pid(mutex,pid);
				bpi->set_base_module_address(mutex,get_base_address(pid));
				is_process_alive = true;

				//send pause signal again to unpause
				if(is_igt) {
					ready = true;
					signal = Signal_split::PAUSE;
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