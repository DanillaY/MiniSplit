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
			signal = NONE;
		}

		~Thread_Manager() {

			for (auto& t : data_readers_threads) {
				if (t.joinable())
					t.join();
			}
			
			if (notifier_thread.joinable())
				notifier_thread.join();
			
		}

		void start_memory_reader_string(char* process_name, uintptr_t offsets[], const char* compared_to, uintptr_t base_module_address, int offsets_len, int pid,Signal_split sig , Thread_Manager* t_manager) {
			std::thread t(&Thread_Manager::read_data_string, t_manager, process_name, offsets, compared_to, base_module_address, offsets_len, pid, sig);
			t.detach();
		}

		void start_memory_reader(char* process_name, uintptr_t offsets[], int buffer,int compared_to, uintptr_t base_module_address, int offsets_len, int pid, Signal_split sig, Thread_Manager* t_manager) {
			std::thread t(&Thread_Manager::read_data, t_manager, process_name, offsets, buffer,compared_to, base_module_address, offsets_len, pid, sig);
			t.detach();
		}
	
		void start_notifier(int argc, char* argv[], Thread_Manager* t_manager) {
			std::cout<<"started notifier" << std::endl;
			std::thread t(&Thread_Manager::notify_on_split, t_manager , argc, argv);
			t.join();
		}

	private:

		void read_data_string(char* process_name, uintptr_t offsets[], const char* compared_to, uintptr_t base_module_address, int offsets_len, int pid, Signal_split sig) {
			
			if(base_module_address != 0 && offsets_len > 0) {
				char* buffer = new char[20]; //buffer is the result of the chain dereference in read_proc_memory

				for(;;){
					buffer = read_proc_memory_string(pid, base_module_address, offsets, offsets_len-1, buffer, 20);
					std::this_thread::sleep_for(std::chrono::milliseconds(5));

					if(strcmp(compared_to, buffer) == 0) {
						std::lock_guard<std::mutex> lock(mutex);
						ready = true;
						signal = sig;
						condition_var.notify_one();
					}

				}
				delete[] buffer;
				buffer = nullptr;
			}
		}

		void read_data(char* process_name, uintptr_t offsets[], int buffer, int compared_to, uintptr_t base_module_address, int offsets_len, int pid, Signal_split sig) {
			
			if(base_module_address != 0 && offsets_len > 0) {

				for(;;){
					int value = read_proc_memory(pid, base_module_address, offsets, offsets_len, buffer);
					std::this_thread::sleep_for(std::chrono::milliseconds(5));
					
					if(compared_to == value) {
						std::lock_guard<std::mutex> lock(mutex);
        				ready = true;
						signal = sig;
						condition_var.notify_one();
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
				
				std::unique_lock<std::mutex> lock(mutex);
				condition_var.wait(lock, [this]() { return ready; });
		
				std::string buffer= To_String(signal);
				boost::system::error_code error;
		
				boost::asio::write(socket, boost::asio::buffer(buffer), error);
		
				//return values to its original state
				signal = NONE;
				ready = false;

				if(error){
		
					std::cerr << "Error while writing data to socket: "<< error.message();
					return -1;
				}
		
				std::this_thread::sleep_for(std::chrono::milliseconds(2));
			}
			catch (std::exception& e)
			{
				std::cerr << e.what() << std::endl;
				return -1;
			}
		
			return 0;
		}
};