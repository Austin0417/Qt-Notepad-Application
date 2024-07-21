#pragma once
#include "ConnectionInfoDialog.h"
#include "ui_HostConnectionInfoDialog.h"
#include "Connection.h"

class HostConnectionInfoDialog : public ConnectionInfoDialog
{
private:
	Ui::HostConnectionInfoDialog ui;
public:
	HostConnectionInfoDialog(const std::unique_ptr<Connection>& connection, QWidget* parent = nullptr);
};

