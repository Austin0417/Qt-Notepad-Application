#pragma once

#include <iostream>
#include <QtWidgets/QMainWindow>
#include <QTextEdit>
#include "ui_Nutpad.h"
#include <QMenuBar>
#include "FileOperationThread.h"
#include <QFileDialog>
#include <QToolButton>
#include <QKeyEvent>
#include <QSyntaxHighlighter>
#include <unordered_set>
#include <unordered_map>
#include <stack>
#include "NutpadTextEdit.h"
#include "EditOperation.h"
#include "UndoTrackerThread.h"
#include "Server.h"
#include "Client.h"
#include "OnlineConnectionThread.h"
#include "OnlineStatusToolButton.h"
#include "TextIndexerThread.h"


class Nutpad : public QMainWindow
{
	Q_OBJECT
private:
	enum class MenuOrder
	{
		FILE,
		EDIT,
		FORMAT,
		ONLINE
	};

	enum class KeyPressAction
	{
		NEW,
		SAVE,
		OPEN,
		SAVE_AS,
		UNDO,
		UNDEFINED
	};

	Ui::NutpadClass ui;
	std::unique_ptr<NutpadTextEdit> notepad_text_;
	std::unique_ptr<QMenuBar> notepad_menu_bar_;
	std::unique_ptr<QFileDialog> file_dialog_;
	std::vector<QMenu*> notepad_menus_;
	std::unique_ptr<OnlineStatusToolButton> online_status_tool_button_;

	// Thread handling file write operations
	FileOperationThread file_operation_thread_;
	QString current_file_destination_;

	// Unordered set to keep track of multi-key press events
	std::unordered_set<int> multi_key_press_events_;

	// Stack for undo action
	std::stack<EditOperation> recent_edit_operations_stack_;
	EditOperation current_edit_operation_;

	// Undo Tracker thread that will constantly run in the background to keep track of undo's
	UndoTrackerThread undo_tracker_thread_;

	// Server and client
	OnlineConnectionThread online_connection_thread_;

	// Text Indexer thread for fast word search/find operations
	TextIndexerThread text_indexer_thread_;

public:
	Nutpad(QWidget* parent = nullptr);
	void keyPressEvent(QKeyEvent* event) override;
	void keyReleaseEvent(QKeyEvent* event) override;
	~Nutpad();

signals:
	void OnCompletedFileRead(const std::string& name_of_file_read, const std::string& input_text);
	void OnClientReceivedTextFromServer(char* host_text);
	void OnClientCursorPositionChanged(const ClientCursorPositionData& cursor_data);
	void OnOnlineConnectionStartSuccess(QAction* host, QAction* join, QAction* terminate);
	void OnClientCharacterRemoved(const ClientRemovedCharacterData& removed_char_data);
	void OnClientTextSelectionReceived(const ClientSelectionData& selection_data);

private:
	void BindActionsToMenus();
	void NewFileWindow();
	void SaveFile();
	void OpenFile();
	void SaveAs();
	void ReadOpenedFile(const std::string& file_location);
	void Undo();
	KeyPressAction CheckMultikeyEvent();
};
