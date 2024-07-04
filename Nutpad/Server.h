#pragma once
#include <boost/asio.hpp>
#include <vector>
#include "IOContextSingleton.h"
#include "ServerToClientHeaders.h"
#include "ClientToServerHeaders.h"
#include "ClientHandleMessageQueue.h"
#include "Connection.h"
#include <iostream>
#include <sstream>
#include <functional>
#include <QString>

template<typename T>
static char* ConvertMessageDataToCharArray(const T& data)
{
	std::stringstream ss;
	ss << data << '\0';

	ss.seekg(0, std::ios::end);
	std::size_t buffer_len = ss.tellg();
	ss.seekg(0, std::ios::beg);

	char* result = new char[buffer_len + 1];
	ss.read(result, sizeof(char) * buffer_len);
	result[buffer_len] = 0;

	return result;
}


using boost::asio::ip::tcp;

class ServerToClientHandle
{
private:
	tcp::socket socket_;
	boost::asio::streambuf read_buffer_;
	boost::asio::streambuf write_buffer_;
	ClientHandleMessageQueue message_queue_;

	int client_id_;
public:
	ServerToClientHandle(int client_id, tcp::socket& socket) : socket_(std::move(socket)), client_id_(client_id)
	{
		Read();
	}
	void Read();

	template<typename T>
	void Write(ServerToClientHeaders header, const T& data, bool should_bypass_queue = false)
	{
		std::ostream os(&write_buffer_);
		os << static_cast<int>(header) << data << '\0';

		if (should_bypass_queue)
		{
			boost::asio::async_write(socket_, write_buffer_, [this](boost::system::error_code ec, std::size_t length)
				{
					if (ec)
					{
						std::cerr << "SERVER: error sending message to client";
					}
				});
		}
		else
		{
			// TODO Place the server-to-client message in the client handle's queue, to send when the client indicates it is ready
			message_queue_.PushMessage(ConvertMessageDataToCharArray(data));
		}

	}
};


class Server : public Connection
{
private:
	//std::string host_ip_address_;
	//short host_port_;
	tcp::acceptor acceptor_;

	std::vector<std::unique_ptr<ServerToClientHandle>> clients_;
	std::function<QString()> get_host_current_text_;

	static int next_client_id_;
public:
	Server(const std::string& ip, short port);
	Server& SetCurrentHostTextCallback(const std::function<QString()>& callback);
	void Start() override;
	void RunIOContext() override;
	void StopIOContext() override;
};

