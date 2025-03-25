#include <boost/asio.hpp>
#include <iostream>

int main() {
	using boost::asio::ip::tcp;

	boost::asio::io_service io_service;
    tcp::resolver resolver(io_service);
    tcp::resolver::query query("127.0.0.1", "8080");
    tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);

    tcp::socket socket(io_service);
    boost::asio::connect(socket, endpoint_iterator);

    std::cout << "connected" << std::endl;

	//TODO create a socket for python, read memory, move socket logic from main, send data to split

	return 0;
}