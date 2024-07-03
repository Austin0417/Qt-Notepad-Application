#pragma once
#include <boost/asio.hpp>
#include "IOContextSingleton.h"
#include "ServerToClientHeaders.h"
#include <iostream>
#include "Connection.h"

using boost::asio::ip::tcp;

class Client : public Connection
{
private:
	tcp::socket socket_;
	tcp::resolver resolver_;
	boost::asio::streambuf read_buffer_;
	boost::asio::streambuf write_buffer_;

	std::string host_ip_address_;
	short host_port_;
	int client_id_;
public:
	Client(const std::string& ip, short port);
	int ClientId() const;
	void Read();

	template<typename T>
	void Write(const T& data)
	{
		std::ostream os(&write_buffer_);
		os << data << '\0';
		boost::asio::async_write(socket_, write_buffer_, [this](boost::system::error_code ec, std::size_t length)
			{
				if (ec)
				{
					std::cerr << "CLIENT: failed to send message to server\n";
				}
			});
	}

	void RunIOContext() override;
	void StopIOContext() override;
};

