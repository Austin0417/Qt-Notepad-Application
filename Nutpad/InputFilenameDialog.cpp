#include "InputFilenameDialog.h"


InputFilenameDialog::InputFilenameDialog(const std::function<void(const QString&, int)>& callback, QWidget* parent) :
	on_filename_inputted_(callback),
	QDialog(parent)
{
	ui.setupUi(this);

	ui.file_type_combo_box->addItem(".txt");
	ui.file_type_combo_box->setCurrentIndex(0);

	connect(ui.filename_line_edit, &QLineEdit::returnPressed, this, [this]()
		{
			if (!ui.filename_line_edit->text().isEmpty())
			{
				on_filename_inputted_(ui.filename_line_edit->text(), ui.file_type_combo_box->currentIndex());
				close();
			}
		});
}
