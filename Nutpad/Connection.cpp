#include "Connection.h"

const std::string& Connection::GetIPAddress() const
{
	return ip_address_;
}

short Connection::GetPortNumber() const
{
	return port_number_;
}

