#pragma once
#include <QTextEdit>
#include <QMouseEvent>
#include <QKeyEvent>
#include <queue>
#include <unordered_set>
#include <functional>


class NutpadTextEdit : public QTextEdit
{
	Q_OBJECT
private:
	bool is_backspace_currently_held_;
	bool is_shift_currently_held_;
	int current_index_of_mouse_cursor_;
	QChar potential_char_to_remove_;
	std::queue<QChar> latest_added_characters_;

	std::unordered_set<int> multi_key_presses_;
	std::function<void()> on_undo_requested_;
public:
	NutpadTextEdit(QWidget* parent = nullptr);
	void SetUndoRequestCallback(const std::function<void()>& callback);
	void mousePressEvent(QMouseEvent* event) override;
	void keyPressEvent(QKeyEvent* event) override;
	void keyReleaseEvent(QKeyEvent* event) override;

	bool IsBackSpaceHeld() const;
	int MouseCursorCurrentIndex() const;
signals:
	void OnMouseLeftClick(int index_in_text_string);
	void OnBackspacePressed();
	void OnBackspaceReleased();
	void OnCharacterRemoved(QChar removed);
	void OnCharacterAdded(QChar added);
};

