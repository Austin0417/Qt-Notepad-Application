#pragma once
#include <boost/asio.hpp>
#include "IOContextSingleton.h"
#include "ServerToClientHeaders.h"
#include "ClientToServerHeaders.h"
#include <iostream>
#include "Connection.h"
#include "ClientPacketParser.h"
#include "NetworkStructures.h"

using boost::asio::ip::tcp;



class Client : public Connection
{
private:
	tcp::socket socket_;
	tcp::resolver resolver_;
	boost::asio::streambuf read_buffer_;
	boost::asio::streambuf write_buffer_;

	//std::string host_ip_address_;
	//short host_port_;
	int client_id_;

	std::function<void(char*)> on_received_text_from_host_;
	std::function<void()> on_client_connect_success_;
	std::function<void(ClientRemovedCharacterData)> on_client_character_removed_;
	std::function<void(ClientSelectionData)> on_selection_data_;
	std::function<void(ClientCursorPositionData)> on_client_cursor_position_changed_;
public:
	Client(const std::string& ip, short port);
	int ClientId() const;
	void Start() override;
	void Read();

	template<typename T>
	void Write(ClientToServerHeaders header, const T& data)
	{
		std::ostream os(&write_buffer_);

		os << static_cast<int>(header) << data << '\0';

		boost::asio::async_write(socket_, write_buffer_, [this](boost::system::error_code ec, std::size_t length)
			{
				if (ec)
				{
					std::cerr << "CLIENT: failed to send message to server\n";
				}
				else
				{
					if (is_terminating_)
					{
						std::cout << "CLIENT: closing client socket due to server termination\n";
						socket_.close();
					}
				}
			});
	}

	template<typename T>
	void WriteSynchronous(ClientToServerHeaders headers, const T& data)
	{
		std::ostream os(&write_buffer_);
		os << static_cast<int>(headers);
		switch (headers)
		{
		case ClientToServerHeaders::CLIENT_END:
		{

			int num_digits_client_id = GetNumDigitsFromInteger(data);
			for (int i = num_digits_client_id; i < ClientTerminationData::NUM_DIGITS_CLIENT_ID; i++)
			{
				os << 0;
			}
			os << data << '\0';
			break;
		}
		default:
		{
			os << data << '\0';
			break;
		}
		}

		std::size_t num_bytes_written = boost::asio::write(socket_, write_buffer_);
	}

	Client& SetOnHostTextReceived(const std::function<void(char*)>& callback);
	Client& SetOnClientConnectSuccess(const std::function<void()>& callback);
	Client& SetOnClientCharacterRemoved(const std::function<void(ClientRemovedCharacterData)>& callback);
	Client& SetSelectionDataCallback(const std::function<void(ClientSelectionData)>& callback);
	Client& SetOnClientCursorPositionChanged(const std::function<void(ClientCursorPositionData)>& callback);
	void Terminate() override;
	void RunIOContext() override;
	void StopIOContext() override;
};

