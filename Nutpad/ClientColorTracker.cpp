#include "ClientColorTracker.h"


void ClientColorTracker::SetClientColor(int client_id, const QColor& color)
{
	client_to_color_mapping_[client_id] = color;
}

void ClientColorTracker::RemoveClient(int client_id)
{
	client_to_color_mapping_.erase(client_id);
}

bool ClientColorTracker::IsColorTaken(const QColor& color) const
{
	for (const auto& entry : client_to_color_mapping_)
	{
		if (entry.second == color)
		{
			return true;
		}
	}
	return false;
}


const QColor& ClientColorTracker::GetClientColor(int client_id) const
{
	if (client_to_color_mapping_.find(client_id) == client_to_color_mapping_.end())
	{
		return QColor();
	}

	return client_to_color_mapping_.at(client_id);
}


