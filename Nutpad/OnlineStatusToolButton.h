#pragma once
#include <QToolButton>
#include <QEvent>
#include <QEnterEvent>
#include <QToolTip>


class OnlineStatusToolButton : public QToolButton
{
	Q_OBJECT
private:
	std::function<QString()> get_tooltip_text_;
public:
	OnlineStatusToolButton(const std::function<QString()>& get_tooltip_text, QWidget* parent = nullptr) : get_tooltip_text_(get_tooltip_text), QToolButton(parent) {}
	void enterEvent(QEnterEvent* event) override;
};

