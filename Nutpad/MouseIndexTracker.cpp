#include "MouseIndexTracker.h"


void MouseIndexTracker::AddNewClient(int new_client_id)
{
	// Initially set the new client's cursor index to 0
	id_to_mouse_index_mapping_[new_client_id] = 0;
}

bool MouseIndexTracker::UpdateIdToIndex(int client_id, int mouse_index)
{
	if (id_to_mouse_index_mapping_.find(client_id) == id_to_mouse_index_mapping_.end())
	{
		return false;
	}

	id_to_mouse_index_mapping_[client_id] = mouse_index;
	return true;
}


void MouseIndexTracker::SetIdToCursorRect(int client_id, const QRect& cursor_rect)
{
	// If client id is already present, update QRect. Otherwise, add it
	id_to_cursor_rect_mapping_[client_id] = cursor_rect;
}


const std::unordered_map<int, int>& MouseIndexTracker::GetIdToMouseIndexMapping() const
{
	return id_to_mouse_index_mapping_;
}

const std::unordered_map<int, QRect>& MouseIndexTracker::GetIdToCursorRectMapping() const
{
	return id_to_cursor_rect_mapping_;
}

