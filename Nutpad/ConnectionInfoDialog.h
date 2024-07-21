#pragma once
#include <QDialog>
#include "Connection.h"


class ConnectionInfoDialog : public QDialog
{
private:
	const std::unique_ptr<Connection>& connection_;

public:
	ConnectionInfoDialog(const std::unique_ptr<Connection>& connection, QWidget* parent = nullptr) : connection_(connection), QDialog(parent) {}
};

