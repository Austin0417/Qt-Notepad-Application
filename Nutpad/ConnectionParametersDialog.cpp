#include "ConnectionParametersDialog.h"

static QString GetHostIpAddress()
{
	QList<QHostAddress> list = QNetworkInterface::allAddresses();

	for (int nIter = 0; nIter < list.count(); nIter++)

	{
		if (!list[nIter].isLoopback())
			if (list[nIter].protocol() == QAbstractSocket::IPv4Protocol)
				return list[nIter].toString();

	}
	return "";
}


ConnectionParametersDialog::ConnectionParametersDialog(const std::function<void(const QString&, short)>& callback, QWidget* parent) :
	on_confirm_(callback),
	QDialog(parent)
{
	ui.setupUi(this);
	ui.ip_address_line_edit->setText(GetHostIpAddress());
	ui.ip_address_line_edit->setReadOnly(true);

	port_line_edit_ = std::make_unique<PortNumberLineEdit>(this);
	port_line_edit_->move(100, 90);
	port_line_edit_->setFixedWidth(113);
	port_line_edit_->setFixedHeight(22);

	connect(ui.confirm_btn, &QPushButton::clicked, this, [this]()
		{

			on_confirm_(ui.ip_address_line_edit->text(), port_line_edit_->text().toShort());
			this->close();
		});
}


void ConnectionParametersDialog::keyPressEvent(QKeyEvent* event)
{
	QDialog::keyPressEvent(event);

	if (event->key() == Qt::Key_Enter && !ui.ip_address_line_edit->text().isEmpty() && !port_line_edit_->text().isEmpty())
	{
		on_confirm_(ui.ip_address_line_edit->text(), port_line_edit_->text().toShort());
		this->close();
	}
}
