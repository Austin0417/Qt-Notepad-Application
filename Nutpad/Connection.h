#pragma once
#include <string>

class Connection
{
protected:
	std::string ip_address_;
	short port_number_;
public:
	Connection(const std::string& ip, short port) : ip_address_(ip), port_number_(port) {}
	const std::string& GetIPAddress() const;
	short GetPortNumber() const;
	virtual void Start() = 0;
	virtual void RunIOContext() = 0;
	virtual void StopIOContext() = 0;
};

