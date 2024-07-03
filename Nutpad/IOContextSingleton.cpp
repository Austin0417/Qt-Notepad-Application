#include "IOContextSingleton.h"


boost::asio::io_context IOContextSingleton::global_server_io_context_;
boost::asio::io_context IOContextSingleton::global_client_io_context_;

boost::asio::io_context& IOContextSingleton::GetServerIOContext()
{
	return global_server_io_context_;
}

boost::asio::io_context& IOContextSingleton::GetClientIOContext()
{
	return global_client_io_context_;
}



