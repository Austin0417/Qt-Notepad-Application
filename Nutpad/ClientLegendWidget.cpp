#include "ClientLegendWidget.h"

static QString GenerateColorString(const QColor& color)
{
	QString result = "rgb(";
	result += QString::number(color.red()) + ", " + QString::number(color.green()) + ", " + QString::number(color.blue()) + ")";
	return result;
}

ClientLegendWidget::ClientLegendWidget(QWidget* parent) : QWidget(parent)
{
	setFixedWidth(146);
	setFixedHeight(34);

	client_name_label_ = std::make_unique<QLabel>(this);
	client_color_label_ = std::make_unique<QLabel>(this);
	client_name_label_->setText("N/A");
	client_color_label_->setStyleSheet("background: black;");

	client_name_label_->setGeometry(20, 10, 49, 16);
	client_color_label_->setGeometry(80, 10, 49, 16);
}


ClientLegendWidget::ClientLegendWidget(const std::string& name, const QColor& color, QWidget* parent) : client_name_(name), client_color_(color), QWidget(parent)
{
	setFixedWidth(146);
	setFixedHeight(34);

	client_name_label_ = std::make_unique<QLabel>(this);
	client_color_label_ = std::make_unique<QLabel>(this);
	client_name_label_->setText(QString::fromStdString(client_name_));
	client_color_label_->setStyleSheet("background: " + GenerateColorString(client_color_));
	client_name_label_->setGeometry(20, 10, 49, 16);
	client_color_label_->setGeometry(80, 10, 49, 16);
}


