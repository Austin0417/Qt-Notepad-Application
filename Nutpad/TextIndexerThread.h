#pragma once
#include <thread>
#include <mutex>
#include <condition_variable>
#include <string>
#include <unordered_map>
#include <functional>
#include <sstream>


class TextIndexerThread
{
private:
	bool active_;
	std::function<void(const std::unordered_map<std::string, std::vector<size_t>>&)> on_token_indexing_complete;

	std::thread text_indexer_thread_;
	std::string target_text_;
	std::mutex mutex_;
	std::condition_variable cv_;
	std::unordered_map<std::string, std::vector<size_t>> token_to_index_mapping_;

public:
	TextIndexerThread(const std::function<void(const std::unordered_map<std::string, std::vector<size_t>>)>& on_token_indexing_complete);
	void SetTargetText(const std::string& target_text);
	const std::unordered_map<std::string, std::vector<size_t>>& GetTokenIndices() const;
	~TextIndexerThread();

private:
	void CreateTokenIndices();
};

