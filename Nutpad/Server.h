#pragma once
#include <boost/asio.hpp>
#include <vector>
#include "IOContextSingleton.h"
#include "ServerToClientHeaders.h"
#include "ClientToServerHeaders.h"
#include "ClientHandleMessageQueue.h"
#include "Connection.h"
#include "ClientPacketParser.h"
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

class Server;

class ServerToClientHandle
{
private:
	tcp::socket socket_;
	boost::asio::streambuf read_buffer_;
	boost::asio::streambuf write_buffer_;
	ClientHandleMessageQueue message_queue_;
	Server* parent_server_;

	int client_id_;

	std::function<void(int)> on_client_terminated_;
	std::function<void(ClientCursorPositionData)> on_client_cursor_changed_;
	std::function<void(ClientRemovedCharacterData)> on_client_character_removed_;
	std::function<void(ClientSelectionData)> on_selection_;
public:
	ServerToClientHandle(Server* parent, int client_id, tcp::socket& socket) : socket_(std::move(socket)), client_id_(client_id), parent_server_(parent)
	{
		Read();
	}
	void Read();
	tcp::socket& GetSocket();
	int GetClientId() const;

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

	template<typename T>
	void WriteSynchronous(ServerToClientHeaders headers, const T& data)
	{
		std::ostream os(&write_buffer_);
		os << static_cast<int>(headers) << data << '\0';

		std::size_t num_bytes_written = boost::asio::write(socket_, write_buffer_);
	}
	ServerToClientHandle& SetOnClientTerminatedCallback(const std::function<void(int)>& callback);
	ServerToClientHandle& SetOnClientCursorChangedCallback(const std::function<void(ClientCursorPositionData)>& callback);
	ServerToClientHandle& SetOnClientCharacterRemovedCallback(const std::function<void(ClientRemovedCharacterData)>& callback);
	ServerToClientHandle& SetClientSelectionDataCallback(const std::function<void(ClientSelectionData)>& callback);
};


class Server : public Connection
{
private:
	//std::string host_ip_address_;
	//short host_port_;
	tcp::acceptor acceptor_;

	std::vector<std::unique_ptr<ServerToClientHandle>> clients_;
	std::function<QString()> get_host_current_text_;
	std::function<void(int)> on_client_join_;
	std::function<void()> on_start_success_;
	std::function<void(const ClientCursorPositionData&)> on_client_cursor_position_changed_;
	std::function<void(ClientSelectionData)> on_client_selection_;
	std::function<void(ClientRemovedCharacterData)> on_client_character_removed_;
	int num_clients_acknowledged_termination_;
	std::condition_variable cv_;
	std::mutex mutex_;

	static int next_client_id_;
public:
	Server(const std::string& ip, short port);
	Server& SetCurrentHostTextCallback(const std::function<QString()>& callback);
	Server& SetOnClientJoinCallback(const std::function<void(int)>& callback);
	Server& SetOnStartSuccessCallback(const std::function<void()>& callback);
	Server& SetOnClientCursorPositionChanged(const std::function<void(const ClientCursorPositionData&)>& callback);
	Server& SetOnClientSelectionCallback(const std::function<void(ClientSelectionData)>& callback);
	Server& SetOnClientCharacterRemoved(const std::function<void(ClientRemovedCharacterData)>& callback);
	void Start() override;

	template<typename T>
	void WriteAllClients(ServerToClientHeaders header, const T& data)
	{
		for (auto& client : clients_)
		{
			client->Write(header, data, true);
		}
	}

	template<typename T>
	void WriteAllClientsSynchronous(ServerToClientHeaders header, const T& data)
	{
		for (auto& client : clients_)
		{
			client->WriteSynchronous(header, data);
		}
	}
	void IncrementNumClientsAcknowledgedTermination();
	void Terminate() override;
	void RunIOContext() override;
	void StopIOContext() override;
};

