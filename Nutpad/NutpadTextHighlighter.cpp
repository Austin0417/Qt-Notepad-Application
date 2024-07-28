#include "NutpadTextHighlighter.h"


void NutpadTextHighlighter::highlightBlock(const QString& text)
{
	const std::unordered_map<int, ClientTextData>& client_data = request_client_data_();
	QTextCharFormat format;

	for (const auto& entry : client_data)
	{
		// First step is to check if the current text block contains any client's text selection
		const ClientTextData& client_text_data = entry.second;
		if (client_text_data.GetSelectionStart() >= currentBlock().position() && client_text_data.GetSelectionEnd() <= currentBlock().position() + text.length())
		{
			// Apply the highlight here
			format.setBackground(QBrush{ client_text_data.GetColor() });
			setFormat(client_text_data.GetSelectionStart() - currentBlock().position(), client_text_data.GetSelectionEnd() - client_text_data.GetSelectionStart(), format);
		}
	}
	/*
	int block_start = currentBlock().position();
	int block_end = block_start + text.length();

	if (block_start >= end_ || block_end <= start_)
	{
		return;
	}

	int translated_block_start = start_ - block_start;


	QTextCharFormat format;
	format.setBackground(QBrush{ current_active_color_ });
	setFormat(translated_block_start, end_ - start_, format);
	*/
}

void NutpadTextHighlighter::SetStart(int start)
{
	start_ = start;
}

void NutpadTextHighlighter::SetEnd(int end)
{
	end_ = end;
}

void NutpadTextHighlighter::SetActiveColor(const QColor& color)
{
	current_active_color_ = color;
}

