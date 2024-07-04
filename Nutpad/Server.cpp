#include "Server.h"


void ServerToClientHandle::Read()
{
	boost::asio::async_read_until(socket_, read_buffer_, '\0', [this](boost::system::error_code ec, std::size_t length)
		{
			if (!ec)
			{
				std::istream is(&read_buffer_);
				char message_header;
				is.read(&message_header, sizeof(char));

				int message_header_int = std::atoi(&message_header);
				switch (message_header_int)
				{
				case static_cast<int>(ClientToServerHeaders::CLIENT_ACKNOWLEDGEMENT):
				{
					std::cout << "SERVER: received acknowledgement from client\n";

					// TODO Pop a message from the queue and send the next message
					if (!message_queue_.IsEmpty())
					{
						// TODO A problem could arise here; since Write is asynchronous and returns immediately,
						// and ClientHandleMessageQueue::PopQueue deallocates the character array,
						// the character array might be deallocated before the write operation completes

						const char* message_to_send = message_queue_.RetrieveNextMessage();
						Write(ServerToClientHeaders::SEND_TEXT, std::string{ message_to_send }, true);
						message_queue_.PopQueue();
					}

					break;
				}
				}
				read_buffer_.consume(length);
				Read();
			}
		});
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


int Server::next_client_id_ = 0;

Server::Server(const std::string& ip, short port) :
	Connection(ip, port),
	acceptor_(IOContextSingleton::GetServerIOContext(), tcp::endpoint(boost::asio::ip::make_address(ip_address_), port_number_))
{
}


Server& Server::SetCurrentHostTextCallback(const std::function<QString()>& callback)
{
	get_host_current_text_ = callback;
	return *this;
}


void Server::Start()
{
	acceptor_.async_accept([this](boost::system::error_code ec, tcp::socket socket)
		{
			if (!ec)
			{
				std::cout << "SERVER: successfully accepted oncoming client connection, id=" << next_client_id_ << "\n";
				clients_.push_back(std::make_unique<ServerToClientHandle>(next_client_id_, socket));

				// Send the client's assigned id to the client
				clients_.back()->Write(ServerToClientHeaders::SEND_ID, next_client_id_, true);
				next_client_id_++;

				// TODO Needs acknowledgement from the client that it has successfully processed the previous message before the server sends the next message
				// TODO IDEA: store messages from the server in a queue, and after receiving an acknoledgement from the client, pop a message from the queue and send it
				// Send the current text of the server/host's notepad to the client

				clients_.back()->Write(ServerToClientHeaders::SEND_TEXT, get_host_current_text_().toStdString());

				Start();
			}
			else
			{
				std::cerr << "SERVER: error accepting oncoming client connection";
			}
		});
}

void Server::RunIOContext()
{
	IOContextSingleton::GetServerIOContext().run();
}


void Server::StopIOContext()
{
	IOContextSingleton::GetServerIOContext().stop();
}