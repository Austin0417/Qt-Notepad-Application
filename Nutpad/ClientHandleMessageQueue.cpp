#include "ClientHandleMessageQueue.h"


void ClientHandleMessageQueue::PushMessage(const char* message)
{
	message_queue_.push(message);
}

const char* ClientHandleMessageQueue::RetrieveNextMessage() const
{
	return message_queue_.front();
}

void ClientHandleMessageQueue::PopQueue()
{
	// Since the message queue owns the messages, it deletes them after being popped from the queue
	delete[] message_queue_.front();

	message_queue_.pop();
}

bool ClientHandleMessageQueue::IsEmpty() const
{
	return message_queue_.empty();
}

