#include "UndoTrackerThread.h"

UndoTrackerThread::UndoTrackerThread(const std::function<void()>& debounce_callback) :
	active_(true),
	undo_tracker_thread_([this]()
		{
			while (active_)
			{
				if (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - time_since_last_edit_).count() >= DEBOUNCE_TIME)
				{
					time_since_last_edit_ = std::chrono::high_resolution_clock::now();
					on_debounce_time_exceeded_();
				}
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
			}
		}),
	time_since_last_edit_(std::chrono::high_resolution_clock::now()),
	on_debounce_time_exceeded_(debounce_callback)

{

}


void UndoTrackerThread::SetTimeOfLastEdit(const std::chrono::steady_clock::time_point& time)
{
	time_since_last_edit_ = time;
}


UndoTrackerThread::~UndoTrackerThread()
{
	active_ = false;
	undo_tracker_thread_.join();
}
