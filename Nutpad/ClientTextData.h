#pragma once
#include <QColor>


class ClientTextData
{
private:
	QColor client_color_;
	int client_current_mouse_index_;
	int client_selection_start_index_;
	int client_selection_end_index_;
	int client_id_;

public:
	ClientTextData() : client_current_mouse_index_(0), client_selection_start_index_(0), client_selection_end_index_(0), client_id_(0) {}
	ClientTextData(int client_id) : client_current_mouse_index_(0), client_selection_start_index_(0), client_selection_end_index_(0), client_id_(client_id) {}
	void SetColor(const QColor& color);
	void SetTextSelection(int start, int end);
	void ClearTextSelection();
	void SetCurrentMouseIndex(int index);
	const QColor& GetColor() const;
	int GetCurrentMouseIndex() const;
	int GetSelectionStart() const;
	int GetSelectionEnd() const;
	std::pair<int, int> GetSelectionRange() const;
};

