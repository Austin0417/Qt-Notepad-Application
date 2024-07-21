#pragma once
#include <unordered_map>
#include <QColor>
#include <optional>


class ClientColorTracker
{
private:
	std::unordered_map<int, QColor> client_to_color_mapping_;
	int latest_client_id_;
public:
	void SetClientColor(int client_id, const QColor& color);
	void RemoveClient(int client_id);
	bool IsColorTaken(const QColor& color) const;
	const QColor& GetClientColor(int client_id) const;
};

