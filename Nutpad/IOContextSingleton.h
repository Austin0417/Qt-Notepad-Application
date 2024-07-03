#pragma once
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

class IOContextSingleton
{
private:
	static boost::asio::io_context global_server_io_context_;
	static boost::asio::io_context global_client_io_context_;

public:
	static boost::asio::io_context& GetServerIOContext();
	static boost::asio::io_context& GetClientIOContext();
};

