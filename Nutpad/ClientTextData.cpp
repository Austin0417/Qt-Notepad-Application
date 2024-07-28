#include "ClientTextData.h"




void ClientTextData::SetColor(const QColor& color)
{
	client_color_ = color;
}

void ClientTextData::SetTextSelection(int start, int end)
{
	client_selection_start_index_ = start;
	client_selection_end_index_ = end;
}

void ClientTextData::ClearTextSelection()
{
	client_selection_start_index_ = -1;
	client_selection_end_index_ = -1;
}

void ClientTextData::SetCurrentMouseIndex(int index)
{
	client_current_mouse_index_ = index;
}

const QColor& ClientTextData::GetColor() const
{
	return client_color_;
}

int ClientTextData::GetCurrentMouseIndex() const
{
	return client_current_mouse_index_;
}

int ClientTextData::GetSelectionStart() const
{
	return client_selection_start_index_;
}

int ClientTextData::GetSelectionEnd() const
{
	return client_selection_end_index_;
}

std::pair<int, int> ClientTextData::GetSelectionRange() const
{
	return std::make_pair(client_selection_start_index_, client_selection_end_index_);
}




