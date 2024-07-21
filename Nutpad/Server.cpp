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

					if (parent_server_->IsTerminating())
					{
						parent_server_->IncrementNumClientsAcknowledgedTermination();
					}

					// TODO Pop a message from the queue and send the next message
					if (!message_queue_.IsEmpty())
					{

						const char* message_to_send = message_queue_.RetrieveNextMessage();
						Write(ServerToClientHeaders::SEND_TEXT, std::string{ message_to_send }, true);
						message_queue_.PopQueue();
					}

					break;
				}
				case static_cast<int>(ClientToServerHeaders::CLIENT_SEND_CURSOR_POS):
				{
					// Cursor position packet structure:
					// Next two characters will hold the client id value
					// Next eight characters after that will hold the cursor index position value
					ClientCursorPositionData cursor_data = GetCursorPositionDataFromStream(is);

					std::cout << "SERVER: received cursor position=" << cursor_data.cursor_position_index_ << " from client id=" << cursor_data.client_id_ << "\n";
					on_client_cursor_changed_(cursor_data);
					break;
				}
				case static_cast<int>(ClientToServerHeaders::CLIENT_END):
				{
					char client_id_char_buffer[ClientTerminationData::NUM_DIGITS_CLIENT_ID + 1];
					is.read(client_id_char_buffer, ClientTerminationData::NUM_DIGITS_CLIENT_ID);
					client_id_char_buffer[ClientTerminationData::NUM_DIGITS_CLIENT_ID] = 0;
					int terminated_client_id = std::atoi(client_id_char_buffer);
					std::cout << "SERVER: received termination from client with id=" << terminated_client_id << "\n";

					// Once we have the id of the client who terminated/disconnected, we need to close this socket, and inform the parent Server object via a callback
					// so that the parent Server can remove the ServerToClientHandle in its vector
					socket_.close();
					on_client_terminated_(client_id_);
					return;
				}
				case static_cast<int>(ClientToServerHeaders::CLIENT_REMOVE_CHAR):
				{
					on_client_character_removed_(GetCharRemovedDataFromStream(is));
					break;
				}
				case static_cast<int>(ClientToServerHeaders::CLIENT_SELECTION_DATA):
				{
					ClientSelectionData selection_data = GetClientSelectionDataFromStream(is);
					on_selection_(selection_data);
					break;
				}
				}
				read_buffer_.consume(length);
				Read();
			}
		});
}


tcp::socket& ServerToClientHandle::GetSocket()
{
	return socket_;
}

int ServerToClientHandle::GetClientId() const
{
	return client_id_;
}

ServerToClientHandle& ServerToClientHandle::SetOnClientTerminatedCallback(const std::function<void(int)>& callback)
{
	on_client_terminated_ = callback;
	return *this;
}

ServerToClientHandle& ServerToClientHandle::SetOnClientCursorChangedCallback(const std::function<void(ClientCursorPositionData)>& callback)
{
	on_client_cursor_changed_ = callback;
	return *this;
}

ServerToClientHandle& ServerToClientHandle::SetOnClientCharacterRemovedCallback(const std::function<void(ClientRemovedCharacterData)>& callback)
{
	on_client_character_removed_ = callback;
	return *this;
}

ServerToClientHandle& ServerToClientHandle::SetClientSelectionDataCallback(const std::function<void(ClientSelectionData)>& callback)
{
	on_selection_ = callback;
	return *this;
}



/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


int Server::next_client_id_ = 0;

Server::Server(const std::string& ip, short port) :
	Connection(ip, port),
	acceptor_(IOContextSingleton::GetServerIOContext(), tcp::endpoint(boost::asio::ip::make_address(ip_address_), port_number_)),
	num_clients_acknowledged_termination_(0)
{
}


Server& Server::SetCurrentHostTextCallback(const std::function<QString()>& callback)
{
	get_host_current_text_ = callback;
	return *this;
}

Server& Server::SetOnClientJoinCallback(const std::function<void(int)>& callback)
{
	on_client_join_ = callback;
	return *this;
}

Server& Server::SetOnStartSuccessCallback(const std::function<void()>& callback)
{
	on_start_success_ = callback;
	return *this;
}

Server& Server::SetOnClientCursorPositionChanged(const std::function<void(const ClientCursorPositionData&)>& callback)
{
	on_client_cursor_position_changed_ = callback;
	return *this;
}

Server& Server::SetOnClientSelectionCallback(const std::function<void(ClientSelectionData)>& callback)
{
	on_client_selection_ = callback;
	return *this;
}


Server& Server::SetOnClientCharacterRemoved(const std::function<void(ClientRemovedCharacterData)>& callback)
{
	on_client_character_removed_ = callback;
	return *this;
}



void Server::Start()
{
	on_start_success_();
	acceptor_.async_accept([this](boost::system::error_code ec, tcp::socket socket)
		{
			if (!ec)
			{
				std::cout << "SERVER: successfully accepted oncoming client connection, id=" << next_client_id_ << "\n";
				std::unique_ptr<ServerToClientHandle> client = std::make_unique<ServerToClientHandle>(this, next_client_id_, socket);
				client->SetOnClientTerminatedCallback([this](int terminated_client_id)
					{
						// TODO Remove the ServerToClientHandle with the given id here
						auto removed_client_it = std::find_if(clients_.begin(), clients_.end(), [this, terminated_client_id](std::unique_ptr<ServerToClientHandle>& client)
							{
								return terminated_client_id == client->GetClientId();
							});
						if (removed_client_it != clients_.end())
						{
							clients_.erase(removed_client_it);
							std::cout << "SERVER: closing server-to-client socket with id=" << terminated_client_id << "\n";
						}
					})
					.SetOnClientCursorChangedCallback([this](ClientCursorPositionData cursor_data)
						{
							// Relay the cursor position index of this particular client to the rest of the clients
							on_client_cursor_position_changed_(cursor_data);

							for (auto& client : clients_)
							{
								if (client->GetClientId() != cursor_data.client_id_)
								{
									client->Write(ServerToClientHeaders::SEND_CLIENT_CURSOR_POS, cursor_data, true);
								}
							}
						})
						.SetOnClientCharacterRemovedCallback([this](ClientRemovedCharacterData removed_char_data)
							{
								on_client_character_removed_(removed_char_data);
								// Send the client's removed character index to the rest of the clients
								for (std::unique_ptr<ServerToClientHandle>& client : clients_)
								{
									if (client->GetClientId() != removed_char_data.client_id_)
									{
										client->Write(ServerToClientHeaders::SERVER_REMOVE_CHAR, removed_char_data, true);
									}
								}
							})
							.SetClientSelectionDataCallback([this](ClientSelectionData selection_data)
								{
									on_client_selection_(selection_data);
									for (auto& client : clients_)
									{
										if (client->GetClientId() != selection_data.client_id_)
										{
											client->Write(ServerToClientHeaders::SEND_SELECTION_DATA, selection_data);
										}
									}
								});
							clients_.push_back(std::move(client));
							on_client_join_(next_client_id_);

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

void Server::IncrementNumClientsAcknowledgedTermination()
{
	num_clients_acknowledged_termination_++;
	cv_.notify_one();
}


void Server::Terminate()
{
	// When terminating the server, we should first notify all of the currently connected clients that the server will shut down
	// This is so that each client can stop their respective sockets that interact with the server
	std::thread([this]()
		{
			is_terminating_ = true;
			WriteAllClients(ServerToClientHeaders::SERVER_END, 1);

			// Lock the thread until all of the clients have acknowledged the server's termination
			std::unique_lock<std::mutex> lock(mutex_);
			cv_.wait(lock, [this]()
				{
					return num_clients_acknowledged_termination_ >= clients_.size();
				});

			// Iterate through the client handles and stop each socket
			for (auto& client : clients_)
			{
				client->GetSocket().close();
			}

			std::cout << "SERVER: terminated successfully\n";

		}).detach();

}

void Server::RunIOContext()
{
	IOContextSingleton::GetServerIOContext().run();
}


void Server::StopIOContext()
{
	IOContextSingleton::GetServerIOContext().stop();
}