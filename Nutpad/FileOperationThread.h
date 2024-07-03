#pragma once
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <fstream>


enum class FileWriteMode
{
	NEW,
	APPEND
};

class FileOperationThread
{
private:
	std::queue<std::string> text_to_write_to_file_;
	std::string file_destination_;
	std::mutex mutex_;
	std::condition_variable cv_;
	FileWriteMode fwm_;
	std::thread file_operation_thread_;

	bool active_;
public:
	FileOperationThread();
	void SetFileWriteDestination(const std::string& file_destination);
	void SendTextToWrite(const std::string& text);
	~FileOperationThread();

private:
	void WriteToFile(const std::string& text);
};

