#include "NutpadTextHighlighter.h"


void NutpadTextHighlighter::highlightBlock(const QString& text)
{
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

