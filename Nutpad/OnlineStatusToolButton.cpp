#include "OnlineStatusToolButton.h"


void OnlineStatusToolButton::enterEvent(QEnterEvent* event)
{
	QToolTip::showText(event->globalPos(), get_tooltip_text_());
}


