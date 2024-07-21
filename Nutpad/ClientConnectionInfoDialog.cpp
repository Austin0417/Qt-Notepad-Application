#include "ClientConnectionInfoDialog.h"


ClientConnectionInfoDialog::ClientConnectionInfoDialog(const std::unique_ptr<Connection>& connection, QWidget* parent) :
	ConnectionInfoDialog(connection, parent)
{
	ui.setupUi(this);

	ui.ip_label->setText("IP: " + QString::fromStdString(connection->GetIPAddress()));
	ui.port_label->setText("Port: " + QString::number(connection->GetPortNumber()));

	connect(ui.ok_btn, &QPushButton::clicked, this, [this]()
		{
			this->close();
		});
}
