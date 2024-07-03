#include "NutpadTextEdit.h"
#include <Windows.h>

NutpadTextEdit::NutpadTextEdit(QWidget* parent) :
	is_backspace_currently_held_(false),
	is_shift_currently_held_(false),
	current_index_of_mouse_cursor_(0),
	QTextEdit(parent)
{
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

	if (event->key() == Qt::Key_Backspace)
	{
		is_backspace_currently_held_ = true;
		emit OnBackspacePressed();
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


bool NutpadTextEdit::IsBackSpaceHeld() const
{
	return is_backspace_currently_held_;
}

int NutpadTextEdit::MouseCursorCurrentIndex() const
{
	return current_index_of_mouse_cursor_;
}


