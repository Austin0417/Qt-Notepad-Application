#pragma once
#include <memory>
#include <QDialog>
#include <QScrollArea>
#include <QVBoxLayout>
#include "ClientLegendWidget.h"


class ViewClientsDialog : public QDialog
{
private:
	std::unique_ptr<QScrollArea> scroll_area_;
	std::vector<std::shared_ptr<ClientLegendWidget>> client_legend_widgets_;

public:
	ViewClientsDialog(QWidget* parent = nullptr);
	ViewClientsDialog(std::vector<std::shared_ptr<ClientLegendWidget>>& legend_widgets, QWidget* parent = nullptr);
	void AddClientLegend(const std::string& client_name, const QColor& client_color);
};

