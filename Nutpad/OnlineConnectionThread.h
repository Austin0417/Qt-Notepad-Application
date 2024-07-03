#pragma once
#include <thread>
#include <memory>
#include "Connection.h"


enum class ConnectionType
{
	HOST,
	CLIENT
};

class OnlineConnectionThread
{
private:
	std::thread online_thread_;
	ConnectionType connection_type_;
	std::unique_ptr<Connection> managed_connection_;
public:
	void StartOnlineConnection(std::unique_ptr<Connection> connection, ConnectionType connection_type);
	ConnectionType GetConnectionType() const;
	std::unique_ptr<Connection>& GetManagedConnection();
	~OnlineConnectionThread();
};

