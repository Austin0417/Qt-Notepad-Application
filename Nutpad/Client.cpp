#include "Client.h"

#include "Server.h"

static char* ParseTextData(std::istream& is)
{
	// Allocate a character buffer with initial size
	int buffer_len = 5000;
	char* buffer = new char[buffer_len];
	int current_buffer_index = 0;

	while (!is.eof())
	{
		if (current_buffer_index >= buffer_len)
		{
			char* tmp = buffer;
			buffer = new char[buffer_len * 2];
			memcpy(buffer, tmp, sizeof(char) * buffer_len);
			delete[] tmp;
			buffer_len *= 2;
		}

		is.read(buffer + current_buffer_index, sizeof(char));
		current_buffer_index++;
	}

	// Null terminate the content buffer
	buffer[current_buffer_index] = 0;

	return buffer;
}


static ServerToClientHeaders GetMessageType(std::istream& is)
{
	char message_type;
	is.read(&message_type, sizeof(char));

	int int_message_type = std::atoi(&message_type);

	return static_cast<ServerToClientHeaders>(int_message_type);
}

Client::Client(const std::string& ip, short port) :
	socket_(IOContextSingleton::GetClientIOContext()),
	resolver_(IOContextSingleton::GetClientIOContext()),
	Connection(ip, port)

{


}

int Client::ClientId() const
{
	return client_id_;
}

void Client::Start()
{
	// Attempt to connect to the server with the given parameters
	boost::asio::async_connect(socket_, resolver_.resolve(ip_address_, std::to_string(port_number_)),
		[this](boost::system::error_code ec, const tcp::endpoint& endpoint)
		{
			if (!ec)
			{
				on_client_connect_success_();
				std::cout << "CLIENT: successfully connected to server\n";
				Read();
			}
			else
			{
				std::cerr << "CLIENT: failed to connect to server\n" << ec.to_string() << "\n";
			}
		});
}

void Client::Read()
{
	boost::asio::async_read_until(socket_, read_buffer_, '\0', [this](boost::system::error_code ec, std::size_t length)
		{
			if (!ec)
			{
				std::istream is(&read_buffer_);


				switch (GetMessageType(is))
				{
				case ServerToClientHeaders::SEND_ID:
				{
					char client_id_char;
					is.read(&client_id_char, sizeof(char));
					client_id_ = std::atoi(&client_id_char);
					std::cout << "CLIENT: successfully assigned id=" << client_id_ << "\n";
					break;
				}
				case ServerToClientHeaders::SEND_TEXT:
				{
					char* contents_buffer = ParseTextData(is);
					std::cout << "CLIENT: received text content from server\n" << contents_buffer;
					on_received_text_from_host_(contents_buffer);
					break;
				}
				case ServerToClientHeaders::SEND_CLIENT_COLOR:
				{
					ClientColorPacket client_color_packet = GetClientColorPacketFromStream(is);
					on_client_color_received_(client_color_packet);
					std::cout << "CLIENT: received client color packet=" << client_color_packet << "\n";
					break;
				}
				case ServerToClientHeaders::SEND_ALL_CLIENT_COLORS:
				{
					std::vector<ClientColorPacket> client_colors = GetAllClientColorsFromStream(is);
					on_all_client_colors_received_(client_colors);
					break;
				}
				case ServerToClientHeaders::SERVER_END:
				{
					is_terminating_ = true;
					break;
				}
				case ServerToClientHeaders::SEND_CLIENT_CURSOR_POS:
				{
					ClientCursorPositionData cursor_data = GetCursorPositionDataFromStream(is);
					std::cout << "CLIENT: received cursor data of client id=" << cursor_data.client_id_ << " with index=" << cursor_data.cursor_position_index_ << "\n";
					on_client_cursor_position_changed_(cursor_data);
					break;
				}
				case ServerToClientHeaders::SEND_SELECTION_DATA:
				{
					ClientSelectionData selection_data = GetClientSelectionDataFromStream(is);
					on_selection_data_(selection_data);
					break;
				}
				case ServerToClientHeaders::SERVER_REMOVE_CHAR:
				{
					ClientRemovedCharacterData removed_char_data = GetCharRemovedDataFromStream(is);
					on_client_character_removed_(removed_char_data);
					std::cout << "CLIENT: received character removed at index=" << removed_char_data.char_index_to_remove_ << " from client with id=" << removed_char_data.client_id_ << "\n";
					break;
				}
				case ServerToClientHeaders::SERVER_REMOVE_SELECTION:
				{
					ClientRemovedSelectionData removed_selection_data = GetClientRemovedSelectionDataFromStream(is);
					on_client_removed_selection_(removed_selection_data);
					break;
				}
				}

				read_buffer_.consume(length);

				// Send an acknowledgement back to the server indicating that this client is ready for the next message
				Write(ClientToServerHeaders::CLIENT_ACKNOWLEDGEMENT, 1);
				Read();
			}
		});
}

Client& Client::SetOnHostTextReceived(const std::function<void(char*)>& callback)
{
	on_received_text_from_host_ = callback;
	return *this;
}

Client& Client::SetOnClientConnectSuccess(const std::function<void()>& callback)
{
	on_client_connect_success_ = callback;
	return *this;
}

Client& Client::SetOnClientColorReceivedCallback(const std::function<void(ClientColorPacket)>& callback)
{
	on_client_color_received_ = callback;
	return *this;
}

Client& Client::SetOnAllClientColorsReceivedCallback(const std::function<void(std::vector<ClientColorPacket>)>& callback)
{
	on_all_client_colors_received_ = callback;
	return *this;
}



Client& Client::SetOnClientCharacterRemoved(const std::function<void(ClientRemovedCharacterData)>& callback)
{
	on_client_character_removed_ = callback;
	return *this;
}

Client& Client::SetSelectionDataCallback(const std::function<void(ClientSelectionData)>& callback)
{
	on_selection_data_ = callback;
	return *this;
}


Client& Client::SetOnClientCursorPositionChanged(const std::function<void(ClientCursorPositionData)>& callback)
{
	on_client_cursor_position_changed_ = callback;
	return *this;
}

Client& Client::SetOnClientRemovedSelection(const std::function<void(ClientRemovedSelectionData)>& callback)
{
	on_client_removed_selection_ = callback;
	return *this;
}



void Client::Terminate()
{
	std::thread([this]()
		{
			// TODO Client should send a message to the server, with a client terminate flag along with their id so that server-side can also terminate the socket to this client
			WriteSynchronous(ClientToServerHeaders::CLIENT_END, client_id_);
			std::cout << "CLIENT: closing client socket with id=" << client_id_ << "\n";
			socket_.close();
			IOContextSingleton::GetClientIOContext().stop();
		}).detach();

}

void Client::RunIOContext()
{
	IOContextSingleton::GetClientIOContext().run();
}

void Client::StopIOContext()
{
	IOContextSingleton::GetServerIOContext().stop();
}