#include "TextIndexerThread.h"


TextIndexerThread::TextIndexerThread(const std::function<void(const std::unordered_map<std::string, std::vector<size_t>>)>& on_token_indexing_complete) :

	active_(true),
	on_token_indexing_complete(on_token_indexing_complete),
	text_indexer_thread_([this]()
		{
			while (active_)
			{
				std::unique_lock<std::mutex> lock(mutex_);
				cv_.wait(lock);

				if (!active_)
				{
					return;
				}

				if (!target_text_.empty())
				{
					CreateTokenIndices();
				}
			}
		})
{

}

void TextIndexerThread::SetTargetText(const std::string& target_text)
{
	target_text_ = target_text;
	cv_.notify_all();
}

const std::unordered_map<std::string, std::vector<size_t>>& TextIndexerThread::GetTokenIndices() const
{
	return token_to_index_mapping_;
}


void TextIndexerThread::CreateTokenIndices()
{
	std::stringstream ss(target_text_);

	std::string current_token;

	while (ss >> current_token)
	{
		if (token_to_index_mapping_.find(current_token) != token_to_index_mapping_.end())
		{
			token_to_index_mapping_[current_token].push_back(ss.tellg());
		}
		else
		{
			token_to_index_mapping_[current_token] = std::vector<size_t>{ static_cast<size_t>(ss.tellg()) };
		}
	}

	on_token_indexing_complete(token_to_index_mapping_);
}

TextIndexerThread::~TextIndexerThread()
{
	active_ = false;
	cv_.notify_all();
	text_indexer_thread_.join();
}