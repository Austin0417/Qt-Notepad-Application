#pragma once
#include <QWidget>
#include <QLabel>


class ClientLegendWidget : public QWidget
{
	Q_OBJECT
private:
	std::string client_name_;
	QColor client_color_;

	// UI Elements
	std::unique_ptr<QLabel> client_name_label_;
	std::unique_ptr<QLabel> client_color_label_;

public:
	ClientLegendWidget(QWidget* parent = nullptr);
	ClientLegendWidget(const std::string& name, const QColor& color, QWidget* parent = nullptr);

};

