#include "NutpadTextEdit.h"
#include <QPainter>
#include <QBrush>
#include <Windows.h>
#include <QToolTip>
#include <iostream>
#include <random>

static QColor GetRandomColor()
{
	static std::random_device dev;
	static std::mt19937 rng(dev());
	static std::uniform_int_distribution<std::mt19937::result_type> dist(50, 255);

	return QColor(dist(rng), dist(rng), dist(rng));
}


NutpadTextEdit::NutpadTextEdit(QWidget* parent) :
	is_backspace_currently_held_(false),
	is_shift_currently_held_(false),
	current_index_of_mouse_cursor_(0),
	QTextEdit(parent),
	client_cursor_thread_([this](const QPoint& point_to_search)
		{
			for (const auto& entry : mit_.GetIdToCursorRectMapping())
			{
				if (entry.second.contains(point_to_search))
				{
					std::cout << "Hover over client=" << entry.first << "\n";
					emit this->OnHoverOverClientCursor(entry.second.center(), entry.first);
					std::this_thread::sleep_for(std::chrono::milliseconds(2000));
				}
			}
		})
{
	/*
	connect(this, &QTextEdit::textChanged, this, [this]()
		{
			if (is_backspace_currently_held_ && current_index_of_mouse_cursor_ > 0)
			{
				emit OnCharacterRemoved(potential_char_to_remove_);
				current_index_of_mouse_cursor_--;
				if (current_index_of_mouse_cursor_ == 0)
				{
					potential_char_to_remove_ = '\0';
					return;
				}
				potential_char_to_remove_ = toPlainText()[current_index_of_mouse_cursor_ - 1];
			}
			else
			{
				while (!latest_added_characters_.empty())
				{
					emit OnCharacterAdded(latest_added_characters_.front());
					latest_added_characters_.pop();
				}
			}
		});
		*/
	text_highlighter_ = new NutpadTextHighlighter(this);
	connect(this, &NutpadTextEdit::OnHoverOverClientCursor, this, [this](const QPoint& pos, int client_id)
		{
			QToolTip::showText(this->mapToGlobal(pos), "Client " + QString::number(client_id), nullptr, QRect{}, 2000);
		});
}

void NutpadTextEdit::SetUndoRequestCallback(const std::function<void()>& callback)
{
	on_undo_requested_ = callback;
}


void NutpadTextEdit::mousePressEvent(QMouseEvent* event)
{
	QTextEdit::mousePressEvent(event);
	if (event->button() == Qt::LeftButton)
	{
		QTextCursor text_cursor = cursorForPosition(event->pos());
		qDebug() << "Mouse left click on text edit at pos=" << QCursor::pos() << ". Current index in string=" << text_cursor.position();
		current_index_of_mouse_cursor_ = text_cursor.position();
		emit OnMouseLeftClick(text_cursor.position());

		if (current_index_of_mouse_cursor_ > 0)
		{
			potential_char_to_remove_ = toPlainText()[current_index_of_mouse_cursor_ - 1];
		}

	}
}

void NutpadTextEdit::mouseReleaseEvent(QMouseEvent* event)
{
	QTextEdit::mouseReleaseEvent(event);

	if (textCursor().hasSelection())
	{
		emit this->OnTextSelection(textCursor().selectionStart(), textCursor().selectionEnd());
	}
}


void NutpadTextEdit::mouseMoveEvent(QMouseEvent* event)
{
	QTextEdit::mouseMoveEvent(event);

	qDebug() << "Text edit mouse move at: " << event->position();
	client_cursor_thread_.SearchPoint(event->position().toPoint());

}

void NutpadTextEdit::keyPressEvent(QKeyEvent* event)
{
	if (event->key() == Qt::Key_Control || event->key() == Qt::Key_Z)
	{
		multi_key_presses_.insert(event->key());
	}

	if (multi_key_presses_.find(Qt::Key_Control) != multi_key_presses_.end() && multi_key_presses_.find(Qt::Key_Z) != multi_key_presses_.end() && on_undo_requested_)
	{
		on_undo_requested_();
		QTextEdit::keyPressEvent(event);
		return;
	}

	if (event->key() == Qt::Key_Backspace && current_index_of_mouse_cursor_ > 0)
	{
		is_backspace_currently_held_ = true;
		if (textCursor().hasSelection())
		{
			emit OnSelectionRemoved(textCursor().selectionStart(), textCursor().selectionEnd());
		}
		else
		{
			emit OnCharacterRemoved(current_index_of_mouse_cursor_);
		}
	}
	else if (event->key() == Qt::Key_Shift)
	{
		is_shift_currently_held_ = true;
	}
	else
	{
		QKeySequence key_sequence(event->key());
		QChar char_key_press = key_sequence.toString()[0];
		if (key_sequence.toString().length() == 1 && (char_key_press.isLetter() || char_key_press.isNumber() || char_key_press.isSymbol()) || char_key_press.isDigit())
		{
			if (GetKeyState(20) && is_shift_currently_held_)
			{
				latest_added_characters_.push(char_key_press.toLower());
			}
			else if (GetKeyState(20) || is_shift_currently_held_)
			{
				latest_added_characters_.push(char_key_press.toUpper());
			}
			else
			{
				latest_added_characters_.push(char_key_press.toLower());
			}
		}
	}
	QTextEdit::keyPressEvent(event);
	current_index_of_mouse_cursor_ = textCursor().position();
}


void NutpadTextEdit::keyReleaseEvent(QKeyEvent* event)
{
	QTextEdit::keyReleaseEvent(event);

	multi_key_presses_.erase(event->key());

	if (event->key() == Qt::Key_Backspace)
	{
		is_backspace_currently_held_ = false;
		emit OnBackspaceReleased();
	}
	else if (event->key() == Qt::Key_Shift)
	{
		is_shift_currently_held_ = false;
	}
}

void NutpadTextEdit::paintEvent(QPaintEvent* event)
{
	QTextEdit::paintEvent(event);

	QPainter painter(viewport());

	// TODO Add logic to make sure the color isn't already taken by another client
	// TODO Add the randomly generated color into the ClientColorTracker obj, associate the client id with the random color

	/*
	QColor random_color = GetRandomColor();
	QPen pen(QBrush(random_color), 1);
	painter.setPen(pen);
	*/
	QPen pen;
	pen.setWidth(1);

	for (const auto& entry : mit_.GetIdToMouseIndexMapping())
	{
		// mit_.GetIdToMouseIndexMapping() returns a const std::unordered_map<int, int>& where the key is the client id, and the value is the client's cursor position in the text editor
		QTextCursor text_cursor(textCursor());
		text_cursor.setPosition(entry.second);
		QRect cursor_rect = cursorRect(text_cursor);

		mit_.SetIdToCursorRect(entry.first, cursor_rect);
		pen.setColor(client_color_tracker_.GetClientColor(entry.first).value());
		painter.setPen(pen);

		painter.drawLine(cursor_rect.topLeft(), cursor_rect.bottomLeft());
	}
}


void NutpadTextEdit::UpdateClientCursorIndex(int client_id, int cursor_pos)
{
	if (!mit_.UpdateIdToIndex(client_id, cursor_pos))
	{
		mit_.AddNewClient(client_id);
	}

	// If the client has not been assigned a color yet, do it here
	if (!client_color_tracker_.GetClientColor(client_id).isValid())
	{
		client_color_tracker_.SetClientColor(client_id, GetRandomColor());
	}

	repaint();
}

void NutpadTextEdit::ApplyClientHighlight(int client_id, int start, int end)
{
	text_highlighter_->SetActiveColor(client_color_tracker_.GetClientColor(client_id));
	text_highlighter_->SetStart(start);
	text_highlighter_->SetEnd(end);

	text_highlighter_->rehighlight();
}



bool NutpadTextEdit::IsBackSpaceHeld() const
{
	return is_backspace_currently_held_;
}

int NutpadTextEdit::MouseCursorCurrentIndex() const
{
	return current_index_of_mouse_cursor_;
}


