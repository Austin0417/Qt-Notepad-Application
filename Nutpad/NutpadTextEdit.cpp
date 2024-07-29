#include "NutpadTextEdit.h"
#include <QPainter>
#include <QBrush>
#include <Windows.h>
#include <QToolTip>
#include <iostream>
#include <random>
#include "ColorHelper.h"


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
	text_highlighter_ = new NutpadTextHighlighter(this, [this]()
		{
			return std::ref(client_data_mapping_);
		});
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

	QPen pen;
	pen.setWidth(1);

	for (const auto& entry : client_data_mapping_)
	{
		QTextCursor text_cursor(textCursor());
		text_cursor.setPosition(entry.second.GetCurrentMouseIndex());
		QRect cursor_rect = cursorRect(text_cursor);

		pen.setColor(entry.second.GetColor());
		painter.setPen(pen);

		painter.drawLine(cursor_rect.topLeft(), cursor_rect.bottomLeft());
	}
}


void NutpadTextEdit::UpdateClientCursorIndex(int client_id, int cursor_pos)
{
	if (client_data_mapping_.find(client_id) == client_data_mapping_.end())
	{
		ClientTextData client_data(client_id);

		// Assigning a color to the new client
		client_data.SetColor(GetRandomColor());

		client_data_mapping_[client_id] = client_data;
	}
	client_data_mapping_[client_id].SetCurrentMouseIndex(cursor_pos);

	// Removing the client's previous text selection highlight (if they were highlighting text)
	client_data_mapping_[client_id].ClearTextSelection();
	text_highlighter_->rehighlight();

	repaint();
}

void NutpadTextEdit::ApplyClientHighlight(int client_id, int start, int end)
{
	if (client_data_mapping_.find(client_id) == client_data_mapping_.end())
	{
		ClientTextData client_data(client_id);
		client_data.SetColor(GetRandomColor());
	}

	client_data_mapping_[client_id].SetTextSelection(start, end);
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

const std::unordered_map<int, ClientTextData>& NutpadTextEdit::GetClientDataMapping() const
{
	return client_data_mapping_;
}

std::unordered_map<int, ClientTextData>& NutpadTextEdit::GetClientDataMapping()
{
	return client_data_mapping_;
}


