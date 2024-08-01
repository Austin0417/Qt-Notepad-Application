#include "ColorHelper.h"
#include <sstream>


QColor GetRandomColor()
{
	static std::random_device dev;
	static std::mt19937 rng(dev());
	static std::uniform_int_distribution<std::mt19937::result_type> dist(50, 255);

	return QColor(dist(rng), dist(rng), dist(rng));
}


std::ostream& operator<<(std::ostream& os, const QColor& color)
{
	os << color.red() << "," << color.green() << "," << color.blue();
	return os;
}


std::ostream& operator<<(std::ostream& os, const ClientColorPacket& client_color_packet)
{
	int client_id_length = GetNumDigitsFromInteger(client_color_packet.client_id_);

	for (int i = client_id_length; i < NUM_DIGITS_CLIENT_ID; i++)
	{
		os << 0;
	}

	os << client_color_packet.client_id_ << client_color_packet.client_color_;

	std::stringstream temp_ss;
	temp_ss << client_color_packet.client_color_;
	temp_ss.seekg(0, std::ios::end);
	int client_color_string_length = temp_ss.tellg();

	// Pad the rest of the color data field so that it is always 11 characters long
	for (int i = client_color_string_length; i < ColorPacket::MAX_LENGTH_COLOR_FIELD; i++)
	{
		os << " ";
	}

	return os;
}

std::ostream& operator<<(std::ostream& os, const std::vector<ClientColorPacket>& client_colors)
{
	for (const auto& client_color : client_colors)
	{
		os << client_color;
	}
	return os;
}