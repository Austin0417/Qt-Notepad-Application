#pragma once
#include <string>


int GetNumDigitsFromInteger(int value);

constexpr int NUM_DIGITS_CLIENT_ID = 2;


namespace ColorPacket
{
	constexpr int MAX_LENGTH_COLOR_FIELD = 11;
}

namespace ClientTerminationData
{
	constexpr int NUM_DIGITS_CLIENT_ID = 2;
}

namespace CursorPositionData
{
	constexpr int NUM_DIGITS_CLIENT_ID_ = 2;
	constexpr int NUM_DIGITS_CURSOR_INDEX_ = 8;
}

namespace SelectionData
{
	constexpr int NUM_DIGITS_CLIENT_ID = 2;
	constexpr int NUM_DIGITS_START_INDEX = 8;
	constexpr int NUM_DIGITS_END_INDEX = 8;
}

namespace CharacterRemovedData
{
	constexpr int NUM_DIGITS_CLIENT_ID_ = 2;
	constexpr int NUM_DIGITS_INDEX_CHAR_REMOVED = 8;
}


class Connection
{
protected:
	std::string ip_address_;
	short port_number_;
	bool is_terminating_;
public:
	Connection(const std::string& ip, short port) : ip_address_(ip), port_number_(port), is_terminating_(false) {}
	const std::string& GetIPAddress() const;
	short GetPortNumber() const;
	virtual void Start() = 0;
	virtual void Terminate() = 0;
	virtual void RunIOContext() = 0;
	virtual void StopIOContext() = 0;
	bool IsTerminating() const;
};

