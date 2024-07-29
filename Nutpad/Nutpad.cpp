#include "Nutpad.h"
#include "ConnectionParametersDialog.h"
#include "InputFilenameDialog.h"
#include "HostConnectionInfoDialog.h"
#include "ClientConnectionInfoDialog.h"
#include "ColorHelper.h"
#include "MessageBox.h"

Nutpad::Nutpad(QWidget* parent) :
	QMainWindow(parent),
	undo_tracker_thread_([this]()
		{
			/*
				if (!current_edit_operation_.GetEditContent().isEmpty())
				{
					recent_edit_operations_stack_.push(current_edit_operation_);
					current_edit_operation_.ResetEditOperation();
				}
				*/
		}),
		text_indexer_thread_([this](const std::unordered_map<std::string, std::vector<size_t>>& token_indices_mapping)
			{

			})
{
	ui.setupUi(this);


	notepad_menu_bar_ = std::make_unique<QMenuBar>(this);
	notepad_menu_bar_->setFixedWidth(this->width() / 2);

	notepad_text_ = std::make_unique<NutpadTextEdit>(this);
	notepad_text_->setFixedWidth(this->width());
	notepad_text_->setFixedHeight(this->height() - notepad_menu_bar_->height());
	notepad_text_->move(0, notepad_menu_bar_->height());
	notepad_text_->setUndoRedoEnabled(false);
	notepad_text_->SetUndoRequestCallback([this]()
		{
			Undo();
		});


	notepad_menus_.push_back(notepad_menu_bar_->addMenu("File"));
	notepad_menus_.push_back(notepad_menu_bar_->addMenu("Edit"));
	notepad_menus_.push_back(notepad_menu_bar_->addMenu("Format"));
	notepad_menus_.push_back(notepad_menu_bar_->addMenu("Online"));

	file_dialog_ = std::make_unique<QFileDialog>(this);
	file_dialog_->setFileMode(QFileDialog::Directory);

	online_status_tool_button_ = std::make_unique<OnlineStatusToolButton>([this]()
		{
			QString result;
			switch (online_connection_thread_.GetConnectionType())
			{
			case ConnectionType::OFFLINE:
			{
				result = "Online Status: You are currently offline";
				break;
			}
			case ConnectionType::CLIENT:
			{
				result = "Online Status: You are currently a client connected to IP=" + QString::fromStdString(online_connection_thread_.GetManagedConnection()->GetIPAddress());
				break;
			}
			case ConnectionType::HOST:
			{
				result = "Online Status: You are currently hosting a notepad on port " + QString::number(online_connection_thread_.GetManagedConnection()->GetPortNumber());
				break;
			}
			}
			return result;
		},
		this);
	online_status_tool_button_->setIcon(QIcon("server_host_icon.jpg"));
	online_status_tool_button_->move(this->width() / 2, online_status_tool_button_->y());
	online_status_tool_button_->setFixedWidth(this->width() / 2);

	view_clients_dialog_ = std::make_unique<ViewClientsDialog>(this);

	connect(file_dialog_.get(), &QFileDialog::fileSelected, this, [this](const QString& directory)
		{

			switch (file_dialog_->fileMode())
			{
			case QFileDialog::Directory:
			{
				// Selecting a directory to write a file
				InputFilenameDialog* dialog = new InputFilenameDialog([this, directory](const QString& inputted_filename, int file_type_selection)
					{
						QString file_destination = directory + "/" + inputted_filename;
						switch (file_type_selection)
						{
						case InputFilenameDialog::TEXT_FILE:
						{
							file_destination += ".txt";
							break;
						}
						}
						qDebug() << "Writing file to location=" << file_destination;
						current_file_destination_ = file_destination;
						file_operation_thread_.SetFileWriteDestination(file_destination.toStdString());
						file_operation_thread_.SendTextToWrite(notepad_text_->toPlainText().toStdString());
						setWindowTitle(inputted_filename);
					});
				dialog->exec();
				break;
			}
			default:
			{
				// Opening an existing file
				ReadOpenedFile(directory.toStdString());
				break;
			}
			}

		});


	connect(this, &Nutpad::OnCompletedFileRead, this, [this](const std::string& input_file_name, const std::string& input_text)
		{
			notepad_text_->setText(QString::fromStdString(input_text));
			setWindowTitle(QString::fromStdString(input_file_name));
		});

	connect(this, &Nutpad::OnOnlineConnectionStartSuccess, this, [this](QAction* host_notepad_action, QAction* join_notepad_action, QAction* view_other_clients, QAction* terminate_connection_action)
		{
			host_notepad_action->setEnabled(false);
			join_notepad_action->setEnabled(false);

			view_other_clients->setEnabled(true);
			terminate_connection_action->setEnabled(true);
		});

	connect(this, &Nutpad::OnClientCharacterRemoved, this, [this](const ClientRemovedCharacterData& removed_data)
		{
			notepad_text_->setText(notepad_text_->toPlainText().remove(removed_data.char_index_to_remove_, 1));
		});

	connect(notepad_text_.get(), &NutpadTextEdit::OnMouseLeftClick, this, [this](int string_index)
		{
			current_edit_operation_.SetIndexOfEdit(string_index);
			if (online_connection_thread_.GetConnectionType() == ConnectionType::CLIENT)
			{
				// If we are currently a client, send the index/position of our QTextCursor to the server
				std::unique_ptr<Connection>& current_connection = online_connection_thread_.GetManagedConnection();
				Client* client = dynamic_cast<Client*>(current_connection.get());

				if (client != nullptr)
				{
					client->Write(ClientToServerHeaders::CLIENT_SEND_CURSOR_POS, ClientCursorPositionData(client->ClientId(), string_index));
				}
			}
			else if (online_connection_thread_.GetConnectionType() == ConnectionType::HOST)
			{
				// If we are the server, send our updated cursor position to the rest of the connected clients
				std::unique_ptr<Connection>& current_connection = online_connection_thread_.GetManagedConnection();
				Server* server = dynamic_cast<Server*>(current_connection.get());

				if (server != nullptr)
				{
					server->WriteAllClients(ServerToClientHeaders::SEND_CLIENT_CURSOR_POS, ClientCursorPositionData(-1, string_index));
				}
			}
		});

	connect(notepad_text_.get(), &NutpadTextEdit::OnTextSelection, this, [this](int start, int end)
		{
			qDebug() << "Text Block selection from " << start << " to " << end;
			switch (online_connection_thread_.GetConnectionType())
			{
			case ConnectionType::HOST:
			{
				Server* server = dynamic_cast<Server*>(online_connection_thread_.GetManagedConnection().get());
				server->WriteAllClients(ServerToClientHeaders::SEND_SELECTION_DATA, ClientRemovedSelectionData{ -1, start, end });
				break;
			}
			case ConnectionType::CLIENT:
			{
				Client* client = dynamic_cast<Client*>(online_connection_thread_.GetManagedConnection().get());
				client->Write(ClientToServerHeaders::CLIENT_SELECTION_DATA, ClientRemovedSelectionData{ client->ClientId(), start, end });
				break;
			}
			default:
			{
				break;
			}
			}
		});

	connect(notepad_text_.get(), &NutpadTextEdit::OnCharacterRemoved, this, [this](int index_of_removed_char)
		{
			qDebug() << "Removed character at index=" << index_of_removed_char;
			std::unique_ptr<Connection>& connection = online_connection_thread_.GetManagedConnection();
			switch (online_connection_thread_.GetConnectionType())
			{
			case ConnectionType::HOST:
			{

				break;
			}
			case ConnectionType::CLIENT:
			{
				Client* client = dynamic_cast<Client*>(connection.get());
				client->Write(ClientToServerHeaders::CLIENT_REMOVE_CHAR, ClientRemovedCharacterData(client->ClientId(), index_of_removed_char));
				break;
			}
			}
		});

	connect(notepad_text_.get(), &NutpadTextEdit::OnSelectionRemoved, this, [this](int start, int end)
		{
			int left = std::min(start, end);
			int right = (left == start) ? end : start;

			std::cout << "Removed selection with start=" << left << " and end=" << right << "\n";

			switch (online_connection_thread_.GetConnectionType())
			{
			case ConnectionType::HOST:
			{
				// We are the host, send the removed text selection data to the rest of our clients
				std::unique_ptr<Connection>& connection = online_connection_thread_.GetManagedConnection();
				Server* server = dynamic_cast<Server*>(connection.get());
				server->WriteAllClients(ServerToClientHeaders::SERVER_REMOVE_SELECTION, ClientRemovedSelectionData{ -1, start, end });
				break;
			}
			case ConnectionType::CLIENT:
			{
				// We are a client, send the removed text selection data to the server; server will send it to the rest of the clients
				std::unique_ptr<Connection>& connection = online_connection_thread_.GetManagedConnection();
				Client* client = dynamic_cast<Client*>(connection.get());
				client->Write(ClientToServerHeaders::CLIENT_REMOVE_SELECTION, ClientRemovedSelectionData{ client->ClientId(), start, end });
				break;
			}
			}
		});

	connect(this, &Nutpad::OnClientReceivedTextFromServer, this, [this](char* host_text)
		{
			notepad_text_->setText(QString{ host_text });
			delete[] host_text;
		});

	connect(online_status_tool_button_.get(), &QToolButton::clicked, this, [this]()
		{
			switch (online_connection_thread_.GetConnectionType())
			{
			case ConnectionType::HOST:
			{
				HostConnectionInfoDialog* dialog = new HostConnectionInfoDialog(online_connection_thread_.GetManagedConnection(), this);
				dialog->show();
				break;
			}
			case ConnectionType::CLIENT:
			{
				ClientConnectionInfoDialog* dialog = new ClientConnectionInfoDialog(online_connection_thread_.GetManagedConnection(), this);
				dialog->show();
				break;
			}
			case ConnectionType::OFFLINE:
			{
				break;
			}
			}
		});

	connect(this, &Nutpad::OnClientCursorPositionChanged, this, [this](const ClientCursorPositionData& cursor_data)
		{
			notepad_text_->UpdateClientCursorIndex(cursor_data.client_id_, cursor_data.cursor_position_index_);
		});

	connect(this, &Nutpad::OnClientTextSelectionReceived, this, [this](const ClientSelectionData& data)
		{
			notepad_text_->ApplyClientHighlight(data.client_id_, data.start_, data.end_);
		});

	connect(this, &Nutpad::OnTextSelectionRemoved, this, [this](const ClientRemovedSelectionData& removed_selection)
		{
			notepad_text_->setText(notepad_text_->toPlainText().remove(removed_selection.start_index_, removed_selection.end_index_ - removed_selection.start_index_));
		});

	BindActionsToMenus();
}

void Nutpad::keyPressEvent(QKeyEvent* event)
{

	if (event->key() == Qt::Key_Control || event->key() == Qt::Key_S || event->key() == Qt::Key_Shift || event->key() == Qt::Key_N || event->key() == Qt::Key_O || event->key() == Qt::Key_Z)
	{
		multi_key_press_events_.insert(event->key());
	}


	switch (CheckMultikeyEvent())
	{
	case KeyPressAction::NEW:
	{
		multi_key_press_events_.clear();
		NewFileWindow();
		break;
	}
	case KeyPressAction::SAVE:
	{
		multi_key_press_events_.clear();
		SaveFile();
		break;
	}
	case KeyPressAction::OPEN:
	{
		multi_key_press_events_.clear();
		OpenFile();
		break;
	}
	case KeyPressAction::SAVE_AS:
	{
		multi_key_press_events_.clear();
		SaveAs();
		break;
	}
	case KeyPressAction::UNDO:
	{
		multi_key_press_events_.clear();
		Undo();
		break;
	}
	}
	QMainWindow::keyPressEvent(event);
}

void Nutpad::keyReleaseEvent(QKeyEvent* event)
{
	QMainWindow::keyReleaseEvent(event);

	multi_key_press_events_.erase(event->key());
}


void Nutpad::BindActionsToMenus()
{
	QMenu* file_menu = notepad_menus_[static_cast<int>(MenuOrder::FILE)];
	QMenu* edit_menu = notepad_menus_[static_cast<int>(MenuOrder::EDIT)];
	QMenu* format_menu = notepad_menus_[static_cast<int>(MenuOrder::FORMAT)];
	QMenu* online_menu = notepad_menus_[static_cast<int>(MenuOrder::ONLINE)];

	QAction* new_notepad_action = file_menu->addAction("New");
	QAction* open_notepad_action = file_menu->addAction("Open...");
	QAction* save_notepad_action = file_menu->addAction("Save");
	QAction* save_as_notepad_action = file_menu->addAction("Save as...");

	QAction* edit_undo_action = edit_menu->addAction("Undo");

	QAction* host_notepad_action = online_menu->addAction("Host");
	QAction* join_notepad_action = online_menu->addAction("Join");
	QAction* view_other_clients_action = online_menu->addAction("View Current Clients");
	QAction* terminate_connection_action = online_menu->addAction("Terminate Connection");

	view_other_clients_action->setEnabled(false);
	terminate_connection_action->setEnabled(false);

	// TODO Connect event handlers for every menu action here
	connect(new_notepad_action, &QAction::triggered, this, [this]()
		{
			NewFileWindow();
		});

	connect(open_notepad_action, &QAction::triggered, this, [this]()
		{
			OpenFile();
		});

	connect(save_notepad_action, &QAction::triggered, this, [this]()
		{
			SaveFile();
		});

	connect(save_as_notepad_action, &QAction::triggered, this, [this]()
		{
			SaveAs();
		});

	connect(edit_undo_action, &QAction::triggered, this, [this]()
		{
			Undo();
		});

	connect(host_notepad_action, &QAction::triggered, this, [this, host_notepad_action, join_notepad_action, view_other_clients_action, terminate_connection_action]()
		{
			ConnectionParametersDialog* dialog = new ConnectionParametersDialog([this, host_notepad_action, join_notepad_action, view_other_clients_action, terminate_connection_action](const QString& server_ip, short port)
				{
					qDebug() << "Starting server with ip=" << server_ip << " on port=" << port;
					std::unique_ptr<Server> server = std::make_unique<Server>(server_ip.toStdString(), port);
					server->SetCurrentHostTextCallback([this]()
						{
							return notepad_text_->toPlainText();
						})
						.SetOnClientJoinCallback([this](int id_of_joined_client)
							{
								//client_cursor_mapping_[id_of_joined_client] = std::make_unique<QTextCursor>(notepad_text_->document());

							})
							.SetOnStartSuccessCallback([this, host_notepad_action, join_notepad_action, view_other_clients_action, terminate_connection_action]()
								{
									emit this->OnOnlineConnectionStartSuccess(host_notepad_action, join_notepad_action, view_other_clients_action, terminate_connection_action);
								})
								.SetOnClientCharacterRemoved([this](ClientRemovedCharacterData removed_char_data)
									{
										emit this->OnClientCharacterRemoved(removed_char_data);
									})
									.SetOnClientCursorPositionChanged([this](const ClientCursorPositionData& cursor_data)
										{
											emit this->OnClientCursorPositionChanged(cursor_data);
										})
										.SetOnClientSelectionCallback([this](ClientSelectionData selection_data)
											{
												std::cout << "SERVER: received selection data from client=" << selection_data.client_id_ << " with start=" << selection_data.start_ << " and end=" << selection_data.end_ << "\n";
												emit this->OnClientTextSelectionReceived(selection_data);
											})
											.SetOnClientSelectionRemoved([this](ClientRemovedSelectionData removed_selection)
												{
													std::cout << "SERVER: " << removed_selection << "\n";
													emit this->OnTextSelectionRemoved(removed_selection);
												});
											online_connection_thread_.StartOnlineConnection(std::move(server), ConnectionType::HOST);

				},
				this);

			dialog->setWindowTitle("Host a Notepad");
			dialog->show();
		});

	connect(join_notepad_action, &QAction::triggered, this, [this, host_notepad_action, join_notepad_action, view_other_clients_action, terminate_connection_action]()
		{
			ConnectionParametersDialog* dialog = new ConnectionParametersDialog([this, host_notepad_action, join_notepad_action, view_other_clients_action, terminate_connection_action](const QString& server_ip, short port)
				{
					qDebug() << "Attempting connection to server with ip=" << server_ip << " on port=" << port;
					std::unique_ptr<Client> client = std::make_unique<Client>(server_ip.toStdString(), port);
					client->SetOnHostTextReceived([this, host_notepad_action, join_notepad_action, view_other_clients_action, terminate_connection_action](char* content_buffer)
						{
							emit this->OnClientReceivedTextFromServer(content_buffer);
						})
						.SetOnClientConnectSuccess([this, host_notepad_action, join_notepad_action, view_other_clients_action, terminate_connection_action]()
							{
								emit this->OnOnlineConnectionStartSuccess(host_notepad_action, join_notepad_action, view_other_clients_action, terminate_connection_action);
							})
							.SetOnClientCharacterRemoved([this](ClientRemovedCharacterData removed_char_data)
								{
									emit this->OnClientCharacterRemoved(removed_char_data);
								})
								.SetOnClientCursorPositionChanged([this](ClientCursorPositionData cursor_data)
									{
										emit this->OnClientCursorPositionChanged(cursor_data);
									})
									.SetSelectionDataCallback([this](ClientSelectionData selection_data)
										{
											std::cout << "CLIENT: received selection data from client=" << selection_data.client_id_ << " with start=" << selection_data.start_ << " and end=" << selection_data.end_ << "\n";
											emit this->OnClientTextSelectionReceived(selection_data);
										})
										.SetOnClientRemovedSelection([this](ClientRemovedSelectionData removed_selection)
											{
												std::cout << "CLIENT: " << removed_selection << "\n";
												emit this->OnTextSelectionRemoved(removed_selection);
											});
										online_connection_thread_.StartOnlineConnection(std::move(client), ConnectionType::CLIENT);

				},
				this);
			dialog->setWindowTitle("Join a Host");
			dialog->show();
		});

	connect(view_other_clients_action, &QAction::triggered, this, [this]()
		{
			// TODO REMOVE LATER (THIS IS ONLY FOR TESTING PURPOSES)
			for (int i = 0; i < 10; i++)
			{
				// Creating ten sample clients to test scroll area functionality
				view_clients_dialog_->AddClientLegend("Client " + std::to_string(i), GetRandomColor());
			}


			view_clients_dialog_->show();
		});

	connect(terminate_connection_action, &QAction::triggered, this, [this, host_notepad_action, join_notepad_action, view_other_clients_action, terminate_connection_action]()
		{
			if (online_connection_thread_.GetConnectionType() != ConnectionType::OFFLINE)
			{
				online_connection_thread_.GetManagedConnection()->Terminate();
				host_notepad_action->setEnabled(true);
				join_notepad_action->setEnabled(true);
				terminate_connection_action->setEnabled(false);
				view_other_clients_action->setEnabled(false);

				online_connection_thread_.ClearConnection();

				ShowOKMessage("Terminate Connection", "Connection was terminated successfully");
			}
		});
}


void Nutpad::NewFileWindow()
{
	qDebug() << "New file window";
	Nutpad* new_main_window = new Nutpad(nullptr);
	new_main_window->show();
}

void Nutpad::SaveFile()
{
	qDebug() << "Save file";
	if (current_file_destination_.isEmpty())
	{
		file_dialog_->exec();
	}
	else
	{
		file_operation_thread_.SetFileWriteDestination(current_file_destination_.toStdString());
		file_operation_thread_.SendTextToWrite(notepad_text_->toPlainText().toStdString());
	}
}

void Nutpad::OpenFile()
{
	file_dialog_->setFileMode(QFileDialog::AnyFile);
	file_dialog_->exec();
}

void Nutpad::ReadOpenedFile(const std::string& file_location)
{
	std::thread([this, file_location]()
		{
			std::ifstream input_file(file_location);
			if (!input_file.is_open())
			{
				std::cerr << "ERROR: failed to open selected file\n";
				return;
			}
			current_file_destination_ = QString::fromStdString(file_location);
			input_file.seekg(0, std::ios::end);
			std::size_t buffer_len = input_file.tellg();
			input_file.seekg(0, std::ios::beg);

			// Obtaining the name of the opened file
			QString qstring_file_location = QString::fromStdString(file_location);
			QStringList split_qstring = qstring_file_location.split("/");
			QString selected_file_name = split_qstring[split_qstring.size() - 1].split(".")[0];

			char* input_buffer = new char[buffer_len + 1];
			memset(input_buffer, 0, buffer_len);

			input_file.read(input_buffer, sizeof(char) * buffer_len);

			emit this->OnCompletedFileRead(selected_file_name.toStdString(), std::string{ input_buffer });

			delete[] input_buffer;

		}).detach();
}

void Nutpad::SaveAs()
{
	qDebug() << "Save as new file";
	file_dialog_->setFileMode(QFileDialog::Directory);
	file_dialog_->exec();
}

void Nutpad::Undo()
{
	if (!current_edit_operation_.GetEditContent().isEmpty())
	{
		recent_edit_operations_stack_.push(current_edit_operation_);
		current_edit_operation_.ResetEditOperation();
	}

	if (recent_edit_operations_stack_.empty())
	{
		return;
	}
	EditOperation& last_operation = recent_edit_operations_stack_.top();
	if (last_operation.GetEditType() == EditOperation::EditType::ADD)
	{
		const QString& text = notepad_text_->toPlainText();
		QString first_half = text.sliced(0, last_operation.GetIndexOfEdit());
		QString second_half = text.sliced(last_operation.GetIndexOfEdit() + last_operation.GetEditContent().length() + 1);

		notepad_text_->setText(first_half + second_half);
	}
	else
	{
		// TODO Account for a delete edit operation
		QString& edit_operation_text = last_operation.GetEditContent();
		std::reverse(edit_operation_text.begin(), edit_operation_text.end());
		QString result_text = notepad_text_->toPlainText();
		result_text.insert(last_operation.GetIndexOfEdit() - last_operation.GetEditContent().length(), edit_operation_text);
		notepad_text_->setText(result_text);
	}

	recent_edit_operations_stack_.pop();
}

Nutpad::KeyPressAction Nutpad::CheckMultikeyEvent()
{
	if (multi_key_press_events_.find(Qt::Key_Control) != multi_key_press_events_.end() && multi_key_press_events_.find(Qt::Key_Shift) != multi_key_press_events_.end() &&
		multi_key_press_events_.find(Qt::Key_N) != multi_key_press_events_.end())
	{
		return KeyPressAction::NEW;
	}
	if (multi_key_press_events_.find(Qt::Key_Control) != multi_key_press_events_.end() && multi_key_press_events_.find(Qt::Key_S) != multi_key_press_events_.end())
	{
		return KeyPressAction::SAVE;
	}
	if (multi_key_press_events_.find(Qt::Key_Control) != multi_key_press_events_.end() && multi_key_press_events_.find(Qt::Key_O) != multi_key_press_events_.end())
	{
		return KeyPressAction::OPEN;
	}
	if (multi_key_press_events_.find(Qt::Key_Control) != multi_key_press_events_.end() && multi_key_press_events_.find(Qt::Key_Shift) != multi_key_press_events_.end() &&
		multi_key_press_events_.find(Qt::Key_S) != multi_key_press_events_.end())
	{
		return KeyPressAction::SAVE_AS;
	}
	if (multi_key_press_events_.find(Qt::Key_Control) != multi_key_press_events_.end() && multi_key_press_events_.find(Qt::Key_Z) != multi_key_press_events_.end())
	{
		return KeyPressAction::UNDO;
	}

	return KeyPressAction::UNDEFINED;
}

Nutpad::~Nutpad()
{


}
