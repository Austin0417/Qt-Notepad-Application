#include "Connection.h"


int GetNumDigitsFromInteger(int value)
{
	std::string str_value = std::to_string(value);

	return str_value.length();
}


const std::string& Connection::GetIPAddress() const
{
	return ip_address_;
}

short Connection::GetPortNumber() const
{
	return port_number_;
}

bool Connection::IsTerminating() const
{
	return is_terminating_;
}

