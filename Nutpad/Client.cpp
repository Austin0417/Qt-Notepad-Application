#include "Client.h"

Client::Client(const std::string& ip, short port) :
	socket_(IOContextSingleton::GetClientIOContext()),
	resolver_(IOContextSingleton::GetClientIOContext()),
	host_ip_address_(ip),
	host_port_(port)

{

	// Attempt to connect to the server with the given parameters
	boost::asio::async_connect(socket_, resolver_.resolve(host_ip_address_, std::to_string(host_port_)),
		[this](boost::system::error_code ec, const tcp::endpoint& endpoint)
		{
			if (!ec)
			{
				std::cout << "CLIENT: successfully connected to server\n";
				Read();
			}
			else
			{
				std::cerr << "CLIENT: failed to connect to server\n";
			}
		});
}

int Client::ClientId() const
{
	return client_id_;
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
					Write("Client with id=" + std::to_string(client_id_) + " is now ready");
					break;
				}
				}
				read_buffer_.consume(length);
				Read();
			}
		});
}

void Client::RunIOContext()
{
	IOContextSingleton::GetClientIOContext().run();
}

void Client::StopIOContext()
{
	IOContextSingleton::GetServerIOContext().stop();
}