#include "Nutpad.h"
#include "ConnectionParametersDialog.h"
#include "InputFilenameDialog.h"

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
			if (online_connection_thread_.GetManagedConnection() == nullptr)
			{
				result = "Online Status: You are currently offline";
			}
			else if (online_connection_thread_.GetConnectionType() == ConnectionType::CLIENT)
			{
				result = "Online Status: You are currently a client connected to IP=" + QString::fromStdString(online_connection_thread_.GetManagedConnection()->GetIPAddress());

			}
			else
			{
				result = "Online Status: You are currently hosting a notepad on port " + QString::number(online_connection_thread_.GetManagedConnection()->GetPortNumber());
			}
			return result;
		},
		this);
	online_status_tool_button_->setIcon(QIcon("server_host_icon.jpg"));
	online_status_tool_button_->move(this->width() / 2, online_status_tool_button_->y());
	online_status_tool_button_->setFixedWidth(this->width() / 2);

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

	connect(notepad_text_.get(), &NutpadTextEdit::OnMouseLeftClick, this, [this](int string_index)
		{
			current_edit_operation_.SetIndexOfEdit(string_index);
		});

	connect(notepad_text_.get(), &NutpadTextEdit::OnCharacterRemoved, this, [this](QChar removed)
		{
			if (current_edit_operation_.GetEditType() == EditOperation::EditType::ADD && !current_edit_operation_.GetEditContent().isEmpty())
			{
				recent_edit_operations_stack_.push(current_edit_operation_);
				current_edit_operation_.SetEditType(EditOperation::EditType::ERASE);
				current_edit_operation_.ResetEditOperation();
			}

			current_edit_operation_.SetEditType(EditOperation::EditType::ERASE);
			current_edit_operation_.AddCharacterToEditContent(removed);

			undo_tracker_thread_.SetTimeOfLastEdit(std::chrono::high_resolution_clock::now());

			qDebug() << "Removed character=" << removed;
		});

	connect(notepad_text_.get(), &NutpadTextEdit::OnCharacterAdded, this, [this](QChar added)
		{
			if (current_edit_operation_.GetEditType() == EditOperation::EditType::ERASE && !current_edit_operation_.GetEditContent().isEmpty())
			{
				recent_edit_operations_stack_.push(current_edit_operation_);
				current_edit_operation_.SetEditType(EditOperation::EditType::ADD);
				current_edit_operation_.ResetEditOperation();
			}

			current_edit_operation_.SetEditType(EditOperation::EditType::ADD);
			current_edit_operation_.AddCharacterToEditContent(added);
			undo_tracker_thread_.SetTimeOfLastEdit(std::chrono::high_resolution_clock::now());

			qDebug() << "Added character=" << added;
		});

	connect(this, &Nutpad::OnClientReceivedTextFromServer, this, [this](char* host_text)
		{
			notepad_text_->setText(QString{ host_text });
			delete[] host_text;
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

	connect(host_notepad_action, &QAction::triggered, this, [this]()
		{
			ConnectionParametersDialog* dialog = new ConnectionParametersDialog([this](const QString& server_ip, short port)
				{
					qDebug() << "Starting server with ip=" << server_ip << " on port=" << port;
					std::unique_ptr<Server> server = std::make_unique<Server>(server_ip.toStdString(), port);
					server->SetCurrentHostTextCallback([this]()
						{
							return notepad_text_->toPlainText();
						});
					online_connection_thread_.StartOnlineConnection(std::move(server), ConnectionType::HOST);

				},
				this);

			dialog->setWindowTitle("Host a Notepad");
			dialog->show();
		});

	connect(join_notepad_action, &QAction::triggered, this, [this]()
		{
			ConnectionParametersDialog* dialog = new ConnectionParametersDialog([this](const QString& server_ip, short port)
				{
					qDebug() << "Attempting connection to server with ip=" << server_ip << " on port=" << port;
					std::unique_ptr<Client> client = std::make_unique<Client>(server_ip.toStdString(), port);
					client->SetOnHostTextReceived([this](char* content_buffer)
						{
							emit this->OnClientReceivedTextFromServer(content_buffer);
						});
					online_connection_thread_.StartOnlineConnection(std::move(client), ConnectionType::CLIENT);

				},
				this);
			dialog->setWindowTitle("Join a Host");
			dialog->show();
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
	qDebug() << "Open file";
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

			// Obtaining the name of the opened file
			QString qstring_file_location = QString::fromStdString(file_location);
			QStringList split_qstring = qstring_file_location.split("/");
			QString selected_file_name = split_qstring[split_qstring.size() - 1].split(".")[0];

			char* input_buffer = new char[500000];
			int next_index_of_input_buffer = 0;
			while (!input_file.eof())
			{
				input_file.read(input_buffer + next_index_of_input_buffer, sizeof(char));
				next_index_of_input_buffer++;
			}
			input_buffer[next_index_of_input_buffer - 1] = 0;

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
