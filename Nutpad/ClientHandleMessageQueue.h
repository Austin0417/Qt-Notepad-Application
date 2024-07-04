#pragma once
#include <queue>


class ClientHandleMessageQueue
{
private:
	std::queue<const char*> message_queue_;
public:
	void PushMessage(const char* message);
	const char* RetrieveNextMessage() const;
	void PopQueue();
	bool IsEmpty() const;
};

