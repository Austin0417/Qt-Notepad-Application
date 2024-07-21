#include "MessageBox.h"
#include <QMessageBox>

void ShowOKMessage(const QString& title, const QString& message)
{
	QMessageBox* message_box = new QMessageBox(nullptr);
	message_box->setWindowTitle(title);
	message_box->setText(message);
	message_box->setIcon(QMessageBox::Information);

	message_box->exec();

	delete message_box;
}

void ShowErrorMessage(const QString& title, const QString& message)
{
	QMessageBox* message_box = new QMessageBox(nullptr);
	message_box->setWindowTitle(title);
	message_box->setText(message);
	message_box->setIcon(QMessageBox::Critical);

	message_box->exec();

	delete message_box;
}
