#pragma once
#include "MouseIndexTracker.h"
#include "ClientColorTracker.h"
#include "NutpadTextHighlighter.h"
#include "ClientTextData.h"
#include <QTextEdit>
#include <QMouseEvent>
#include <QKeyEvent>
#include <queue>
#include <unordered_set>
#include <condition_variable>
#include <mutex>
#include <functional>
#include <thread>


class NutpadTextEdit : public QTextEdit
{
	Q_OBJECT
private:
	class ClientCursorThread
	{
	private:
		std::mutex mutex_;
		std::condition_variable cv_;
		std::function<void(const QPoint&)> callback_;
		std::thread client_cursor_checker_thread_;

		QPoint current_point_to_search_;

	public:
		ClientCursorThread(const std::function<void(const QPoint&)>& callback) : callback_(callback), client_cursor_checker_thread_([this]()
			{
				while (true)
				{
					callback_(current_point_to_search_);
				}
			})
		{
			client_cursor_checker_thread_.detach();
		}
		void SearchPoint(const QPoint& point)
		{
			current_point_to_search_ = point;
			cv_.notify_all();
		}
	};
private:
	bool is_backspace_currently_held_;
	bool is_shift_currently_held_;
	int current_index_of_mouse_cursor_;
	QChar potential_char_to_remove_;
	std::queue<QChar> latest_added_characters_;

	MouseIndexTracker mit_;
	ClientColorTracker client_color_tracker_;
	std::unordered_map<int, ClientTextData> client_data_mapping_;

	std::unordered_set<int> multi_key_presses_;
	std::function<void()> on_undo_requested_;

	ClientCursorThread client_cursor_thread_;

	NutpadTextHighlighter* text_highlighter_;
public:
	NutpadTextEdit(QWidget* parent = nullptr);
	void SetUndoRequestCallback(const std::function<void()>& callback);
	void mousePressEvent(QMouseEvent* event) override;
	void mouseReleaseEvent(QMouseEvent* event) override;
	void mouseMoveEvent(QMouseEvent* event) override;
	void keyPressEvent(QKeyEvent* event) override;
	void keyReleaseEvent(QKeyEvent* event) override;
	void paintEvent(QPaintEvent* event) override;

	void UpdateClientCursorIndex(int client_id, int cursor_pos);
	void ApplyClientHighlight(int client_id, int start, int end);

	bool IsBackSpaceHeld() const;
	int MouseCursorCurrentIndex() const;

	const std::unordered_map<int, ClientTextData>& GetClientDataMapping() const;
	std::unordered_map<int, ClientTextData>& GetClientDataMapping();
signals:
	void OnMouseLeftClick(int index_in_text_string);
	void OnCharacterRemoved(int index_of_removed_char);
	void OnSelectionRemoved(int start_index, int end_index);
	void OnBackspaceReleased();
	void OnTextSelection(int start, int end);
	void OnHoverOverClientCursor(const QPoint& pos, int client_id);
	/*
	void OnCharacterRemoved(QChar removed);
	void OnCharacterAdded(QChar added);
	*/
};

