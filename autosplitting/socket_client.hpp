#include <iostream>
#include <boost/array.hpp>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

int notify_on_split(int argc, char* argv[]) {
    try
    {
        if (argc != 2)
        {
            std::cerr << "No server ip is given" << std::endl;
            return 1;
        }

        boost::asio::io_context io_context;

        tcp::resolver resolver(io_context);
        tcp::resolver::results_type endpoints = resolver.resolve(argv[1], "5554");

        tcp::socket socket(io_context);
        boost::asio::connect(socket, endpoints);

        for (;;)
        {
            std::string buf="hello :)";
            boost::system::error_code error;

            size_t bytes = boost::asio::write(socket, boost::asio::buffer(buf), error);

            if(error){

                std::cout << "Error while writing data to socket: "<< error.message();
                return -1;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}