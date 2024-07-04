#include "OnlineConnectionThread.h"

OnlineConnectionThread::OnlineConnectionThread() :
	connection_type_(ConnectionType::HOST),
	managed_connection_(nullptr)
{

}


void OnlineConnectionThread::StartOnlineConnection(std::unique_ptr<Connection> connection, ConnectionType connection_type)
{
	connection_type_ = connection_type;
	managed_connection_ = std::move(connection);
	online_thread_ = std::thread([this]()
		{
			managed_connection_->Start();
			managed_connection_->RunIOContext();
		});
}


ConnectionType OnlineConnectionThread::GetConnectionType() const
{
	return connection_type_;
}

std::unique_ptr<Connection>& OnlineConnectionThread::GetManagedConnection()
{
	return managed_connection_;
}

OnlineConnectionThread::~OnlineConnectionThread()
{
	managed_connection_->StopIOContext();
	online_thread_.join();
}