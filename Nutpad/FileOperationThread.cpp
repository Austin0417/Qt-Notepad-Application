#include "FileOperationThread.h"


FileOperationThread::FileOperationThread() :
	active_(true),
	fwm_(FileWriteMode::NEW),
	file_operation_thread_([this]()
		{
			while (active_)
			{
				std::unique_lock<std::mutex> lock(mutex_);
				cv_.wait(lock, [this]()
					{
						return !text_to_write_to_file_.empty() || !active_;
					});

				if (text_to_write_to_file_.empty() && !active_)
				{
					return;
				}

				if (!file_destination_.empty() && !text_to_write_to_file_.empty())
				{
					WriteToFile(text_to_write_to_file_.front());
					text_to_write_to_file_.pop();
				}
			}
		})
{

}

void FileOperationThread::SetFileWriteDestination(const std::string& file_destination)
{
	file_destination_ = file_destination;
}

void FileOperationThread::SendTextToWrite(const std::string& text)
{
	std::lock_guard<std::mutex> lock(mutex_);
	text_to_write_to_file_.push(text);
	cv_.notify_one();
}


void FileOperationThread::WriteToFile(const std::string& text)
{
	switch (fwm_)
	{
	case FileWriteMode::NEW:
	{
		std::ofstream output(file_destination_);
		output.write(text.c_str(), text.length());
		output.flush();
		break;
	}
	case FileWriteMode::APPEND:
	{
		std::ofstream output(file_destination_, std::ios::app);
		output.write(text.c_str(), text.length());
		output.flush();
		break;
	}
	}

}

FileOperationThread::~FileOperationThread()
{
	active_ = false;
	cv_.notify_one();
	file_operation_thread_.join();
}