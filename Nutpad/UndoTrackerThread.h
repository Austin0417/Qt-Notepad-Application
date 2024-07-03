#pragma once
#include <thread>
#include <chrono>
#include <functional>


constexpr int DEBOUNCE_TIME = 2000;

class UndoTrackerThread
{
private:
	bool active_;
	std::thread undo_tracker_thread_;
	std::chrono::steady_clock::time_point time_since_last_edit_;
	std::function<void()> on_debounce_time_exceeded_;
public:
	UndoTrackerThread(const std::function<void()>& debounce_callback);
	void SetTimeOfLastEdit(const std::chrono::steady_clock::time_point& time);
	~UndoTrackerThread();
};

