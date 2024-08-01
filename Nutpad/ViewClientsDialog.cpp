#include "ViewClientsDialog.h"

ViewClientsDialog::ViewClientsDialog(QWidget* parent) : QDialog(parent)
{
	setFixedWidth(415);
	setFixedHeight(300);

	scroll_area_ = std::make_unique<QScrollArea>(this);
	scroll_area_->setFixedWidth(281);
	scroll_area_->setFixedHeight(221);
	scroll_area_->move(70, 40);
	scroll_area_->setWidgetResizable(true);

	QWidget* scroll_area_central_widget = new QWidget();
	/*
	scroll_area_central_widget->setFixedWidth(scroll_area_->width());
	scroll_area_central_widget->setFixedHeight(scroll_area_->height());
	*/
	scroll_area_central_widget->setLayout(new QVBoxLayout());
	scroll_area_->setWidget(scroll_area_central_widget);

}

ViewClientsDialog::ViewClientsDialog(std::vector<std::shared_ptr<ClientLegendWidget>>& legend_widgets, QWidget* parent) :
	client_legend_widgets_(std::move(legend_widgets)),
	QDialog(parent)
{
	setFixedWidth(415);
	setFixedHeight(300);

	scroll_area_ = std::make_unique<QScrollArea>(this);
	scroll_area_->setFixedWidth(281);
	scroll_area_->setFixedHeight(221);
	scroll_area_->move(70, 40);
	scroll_area_->setWidgetResizable(true);

	QWidget* scroll_area_central_widget = new QWidget();
	/*
	scroll_area_central_widget->setFixedWidth(scroll_area_->width());
	scroll_area_central_widget->setFixedHeight(scroll_area_->height());
	*/
	scroll_area_central_widget->setLayout(new QVBoxLayout());
	scroll_area_->setWidget(scroll_area_central_widget);
}

void ViewClientsDialog::AddClientLegend(const std::string& client_name, const QColor& client_color)
{
	client_legend_widgets_.push_back(std::make_shared<ClientLegendWidget>(client_name, client_color, scroll_area_.get()));
	if (scroll_area_->widget() != nullptr && scroll_area_->widget()->layout() != nullptr)
	{
		QVBoxLayout* vbox_layout = dynamic_cast<QVBoxLayout*>(scroll_area_->widget()->layout());
		if (vbox_layout != nullptr)
		{
			vbox_layout->addWidget(client_legend_widgets_.back().get());

		}
	}
}

void ViewClientsDialog::ClearLegend()
{
	if (!client_legend_widgets_.empty() && scroll_area_->widget() != nullptr && scroll_area_->widget()->layout() != nullptr)
	{
		QLayout* layout = scroll_area_->widget()->layout();
		for (const auto& client_legend : client_legend_widgets_)
		{
			layout->removeWidget(client_legend.get());
		}
	}
	client_legend_widgets_.clear();
}
