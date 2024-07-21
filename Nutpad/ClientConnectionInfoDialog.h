#pragma once
#include "ConnectionInfoDialog.h"
#include "ui_ClientConnectionInfoDialog.h"


class ClientConnectionInfoDialog : public ConnectionInfoDialog
{
private:
	Ui::ClientConnectionInfoDialog ui;

public:
	ClientConnectionInfoDialog(const std::unique_ptr<Connection>& connection, QWidget* parent = nullptr);
};

