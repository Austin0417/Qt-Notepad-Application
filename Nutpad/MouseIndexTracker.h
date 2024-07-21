#pragma once
#include <unordered_map>
#include <QRect>


class MouseIndexTracker
{
private:
	std::unordered_map<int, int> id_to_mouse_index_mapping_;
	std::unordered_map<int, QRect> id_to_cursor_rect_mapping_;

public:
	// This will be called everytime a new client joins
	void AddNewClient(int new_client_id);

	// Updates the mouse cursor index for the given client with client_id
	bool UpdateIdToIndex(int client_id, int mouse_index);

	void SetIdToCursorRect(int client_id, const QRect& cursor_rect);

	const std::unordered_map<int, int>& GetIdToMouseIndexMapping() const;
	const std::unordered_map<int, QRect>& GetIdToCursorRectMapping() const;
};

