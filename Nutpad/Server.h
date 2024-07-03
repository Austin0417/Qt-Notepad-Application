#pragma once
#include <boost/asio.hpp>
#include <vector>
#include "IOContextSingleton.h"
#include "ServerToClientHeaders.h"
#include "Connection.h"
#include <iostream>

using boost::asio::ip::tcp;

class ServerToClientHandle
{
private:
	tcp::socket socket_;
	boost::asio::streambuf read_buffer_;
	boost::asio::streambuf write_buffer_;
	int client_id_;
public:
	ServerToClientHandle(int client_id, tcp::socket& socket) : socket_(std::move(socket)), client_id_(client_id)
	{
		Read();
	}
	void Read();

	template<typename T>
	void Write(ServerToClientHeaders header, const T& data)
	{
		std::ostream os(&write_buffer_);
		os << static_cast<int>(header) << data << '\0';

		boost::asio::async_write(socket_, write_buffer_, [this](boost::system::error_code ec, std::size_t length)
			{
				if (ec)
				{
					std::cerr << "SERVER: error sending message to client";
				}
			});
	}
};


class Server : public Connection
{
private:
	std::string host_ip_address_;
	short host_port_;
	tcp::acceptor acceptor_;

	std::vector<std::unique_ptr<ServerToClientHandle>> clients_;

	static int next_client_id_;
public:
	Server(const std::string& ip, short port);
	void RunIOContext() override;
	void StopIOContext() override;
private:
	void StartAccept();
};

