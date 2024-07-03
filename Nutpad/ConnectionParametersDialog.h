#pragma once
#include "ui_ConnectionParametersDialog.h"
#include <QDialog>
#include <QKeyEvent>
#include <QHostAddress>
#include <QNetworkInterface>
#include <functional>

class PortNumberLineEdit : public QLineEdit
{
	Q_OBJECT
public:
	PortNumberLineEdit(QWidget* parent = nullptr) : QLineEdit(parent) {}
	void keyPressEvent(QKeyEvent* event) override
	{
		if (event->key() == Qt::Key_Backspace)
		{
			QLineEdit::keyPressEvent(event);
			return;
		}

		if (event->key() >= Qt::Key_Space && event->key() <= Qt::Key_AsciiTilde)
		{
			QChar key_char(event->key());
			qDebug() << key_char;
			if (key_char.isDigit() || key_char.isNumber())
			{
				QLineEdit::keyPressEvent(event);
			}
		}
	}
};

class ConnectionParametersDialog : public QDialog
{
	Q_OBJECT
private:
	Ui::ConnectionParametersDialog ui;
	std::function<void(const QString&, short)> on_confirm_;
	std::unique_ptr<PortNumberLineEdit> port_line_edit_;
public:
	ConnectionParametersDialog(const std::function<void(const QString&, short)>& callback, QWidget* parent = nullptr);
	void keyPressEvent(QKeyEvent* event) override;
};

