#pragma once
#include "ui_InputFilenameDialog.h"



class InputFilenameDialog : public QDialog
{
private:
	Ui::InputFilenameDialog ui;
	std::function<void(const QString&, int)> on_filename_inputted_;

public:
	InputFilenameDialog(const std::function<void(const QString&, int)>& callback, QWidget* parent = nullptr);

	enum FileTypeSelection
	{
		TEXT_FILE
	};
};

