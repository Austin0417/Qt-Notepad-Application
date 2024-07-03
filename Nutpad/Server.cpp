#include "Server.h"


void ServerToClientHandle::Read()
{
	boost::asio::async_read_until(socket_, read_buffer_, '\0', [this](boost::system::error_code ec, std::size_t length)
		{
			if (!ec)
			{
				std::istream is(&read_buffer_);
				char* temp = new char[length + 1];
				is.read(temp, length);
				temp[length] = 0;

				std::cout << "SERVER: read from client=" << temp << "\n";
				delete[] temp;
				Read();
			}
		});
}



/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



int Server::next_client_id_ = 0;

Server::Server(const std::string& ip, short port) :
	host_ip_address_(ip),
	host_port_(port),
	acceptor_(IOContextSingleton::GetServerIOContext(), tcp::endpoint(boost::asio::ip::make_address(host_ip_address_), host_port_))
{
	StartAccept();
}


void Server::StartAccept()
{
	acceptor_.async_accept([this](boost::system::error_code ec, tcp::socket socket)
		{
			if (!ec)
			{
				std::cout << "SERVER: successfully accepted oncoming client connection, id=" << next_client_id_ << "\n";
				clients_.push_back(std::make_unique<ServerToClientHandle>(next_client_id_, socket));
				clients_.back()->Write(ServerToClientHeaders::SEND_ID, next_client_id_);
				next_client_id_++;

				StartAccept();
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