#include "Client.h"

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

				// Determine the type of message sent by the server
				char message_type;
				is.read(&message_type, sizeof(char));

				int message_type_int = std::atoi(&message_type);
				switch (message_type_int)
				{
				case static_cast<int>(ServerToClientHeaders::SEND_ID):
				{
					char client_id_char;
					is.read(&client_id_char, sizeof(char));
					client_id_ = std::atoi(&client_id_char);
					std::cout << "CLIENT: successfully assigned id=" << client_id_ << "\n";
					break;
				}
				case static_cast<int>(ServerToClientHeaders::SEND_TEXT):
				{
					char* contents_buffer = ParseTextData(is);
					std::cout << "CLIENT: received text content from server\n" << contents_buffer;
					on_received_text_from_host_(contents_buffer);
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


void Client::RunIOContext()
{
	IOContextSingleton::GetClientIOContext().run();
}

void Client::StopIOContext()
{
	IOContextSingleton::GetServerIOContext().stop();
}