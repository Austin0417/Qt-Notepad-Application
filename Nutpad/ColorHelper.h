#pragma once
#include <QColor>
#include <random>
#include "Connection.h"
#include <iostream>

struct ClientColorPacket
{
	int client_id_;
	QColor client_color_;
	bool is_valid_;
	ClientColorPacket() : is_valid_(false) {}
	ClientColorPacket(int client_id, const QColor& client_color) : client_id_(client_id), client_color_(client_color), is_valid_(true) {}
};


QColor GetRandomColor();

std::ostream& operator<<(std::ostream& os, const QColor& color);

std::ostream& operator<<(std::ostream& os, const ClientColorPacket& client_color_packet);

std::ostream& operator<<(std::ostream& os, const std::vector<ClientColorPacket>& client_colors);