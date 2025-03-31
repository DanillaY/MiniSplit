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

        void start_memory_reader_string(std::shared_ptr<Basic_Process_Info> bpi, const char* compared_to, bool with_change_back, char* buffer, unsigned int offsets_len,unsigned int char_len, uintptr_t* offsets,Signal_split sig ,Thread_Manager* t_manager) {
			data_readers_threads.push_back(std::thread(&Thread_Manager::read_data_string, t_manager, compared_to, buffer, offsets_len, offsets,char_len,sig, bpi, with_change_back));
			data_readers_threads.back().detach();
			
        }

        void start_memory_reader(std::shared_ptr<Basic_Process_Info> bpi, int buffer, int compared_to, bool with_change_back, uintptr_t* offsets, unsigned int offsets_len, Signal_split sig, Thread_Manager* t_manager) {
			{
				std::lock_guard lock(mutex);
				data_readers_threads.push_back(std::thread(&Thread_Manager::read_data, t_manager, buffer,compared_to, bpi->get_base_module_address(mutex), bpi->get_pid(mutex), offsets, offsets_len, sig,with_change_back));
				data_readers_threads.back().detach();
			}
        }
    
        void start_notifier(int argc, char* argv[], Thread_Manager* t_manager) {
            std::cout<<"Started notifier" << std::endl;
            notifier_thread= std::thread(&Thread_Manager::notify_on_split, t_manager , argc, argv);
			notifier_thread.join();
        }

		void start_listen_active_process_terminate(std::shared_ptr<Basic_Process_Info> bpi, std::vector<std::function<void ()>> all_functions, Thread_Manager* t_manager) {
			std::thread t(&Thread_Manager::listen_active_process_terminate,t_manager, bpi , all_functions);
			t.detach();
			
		}

    private:

        //with_change_back param is used to send the command twice. once it detects that the current value has changed to the desired one, it sends the signal again after the current value changes, this is useful for tracking things like in-game time, where you can send a PAUSE signal and then unpause
        void read_data_string(const char* compared_to, char* buffer, unsigned int offsets_len, uintptr_t* offsets, unsigned int char_len,Signal_split sig, std::shared_ptr<Basic_Process_Info> bpi, bool with_change_back) {
            
			int pid = 0;
			uintptr_t base_module_address = 0;
			{
				//check if the base module and an array of offsets is valid
				std::lock_guard lock(mutex);
				if(bpi->get_base_module_address(mutex) == 0 || offsets_len <= 0) {
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
				std::strcpy(current_value, read_proc_memory_string(pid, base_module_address, offsets, offsets_len-1, char_len, buffer, 20));
				
				std::this_thread::sleep_for(std::chrono::milliseconds(4));
				bool change_back = (strcmp(previous_value, compared_to) != 0 && strcmp(current_value, compared_to) == 0) || (strcmp(current_value, compared_to) != 0 && strcmp(previous_value, compared_to) == 0);
				
				if(with_change_back == true ? change_back : (strcmp(compared_to, current_value) == 0 && strcmp(current_value, previous_value) != 0 && sig != Signal_split::NONE)) {
					{
						std::lock_guard lock(mutex);
						ready = true;
						signal = sig;
						condition_var.notify_one();
					}
				}
			}
			
        }

        void read_data(int buffer, int compared_to, uintptr_t base_module_address,int pid, uintptr_t* offsets, unsigned int offsets_len, Signal_split sig, bool with_change_back) {

            if(base_module_address != 0 && offsets_len > 0) {
                int current_value = 0;
                int previous_value = 0;

                while(is_process_alive) {
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
                
                while(is_process_alive) {
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

		void listen_active_process_terminate(std::shared_ptr<Basic_Process_Info> bpi, std::vector<std::function<void()>> all_functions) {

			std::string proc_name = "";
			int pid=0;
			{
				std::lock_guard lock(mutex);
				proc_name = bpi->get_process_name(mutex);
				pid = get_process_id_by_name(proc_name);
				std::cout << "Process alive: " << pid << std::endl;
			}

			while(is_process_alive) {
				pid = get_process_id_by_name(proc_name);

				if(pid == 0) {
					break;
				}
				std::this_thread::sleep_for(std::chrono::seconds(2));
			}

			{
				std::lock_guard lock(mutex);
				std::cout << bpi->get_process_name(mutex) << " process was terminated" << std::endl;
				bool ready = false;
				bpi->set_base_module_address(mutex,0);
				bpi->set_pid(mutex,0);
				is_process_alive = false;
			}
			
			listen_active_process_alive(bpi,all_functions,proc_name);
		}

		void listen_active_process_alive(std::shared_ptr<Basic_Process_Info> bpi, std::vector<std::function<void()>> all_functions, std::string proc_name) { 

			//start listening for a new process
			while(is_process_alive == false) {

				int pid = get_process_id_by_name(proc_name);

				if(pid != 0) {
					{
						std::lock_guard lock(mutex);
						bpi->set_pid(mutex,pid);
						bpi->set_base_module_address(mutex,get_base_address(pid));
					}
				
					std::cout<< proc_name <<" was found with pid: " << pid << std::endl;
					is_process_alive = true;

					//we start all the memory reading and notifying functions again after the proccess was found
					for (auto& minispli_function : all_functions) {
						minispli_function();
					}
				}
				std::this_thread::sleep_for(std::chrono::seconds(2));
			}

			//start listening if the process is closed again
			listen_active_process_terminate(bpi, all_functions);
		}
};