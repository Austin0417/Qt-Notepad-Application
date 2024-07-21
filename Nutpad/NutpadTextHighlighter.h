#pragma once
#include <QSyntaxHighlighter>
#include <QTextEdit>


class NutpadTextHighlighter : public QSyntaxHighlighter
{
private:
	int start_;
	int end_;
	QColor current_active_color_;
public:
	NutpadTextHighlighter(QTextDocument* text_document) : QSyntaxHighlighter(text_document), start_(0), end_(0) {}
	NutpadTextHighlighter(QTextEdit* text_edit) : QSyntaxHighlighter(text_edit), start_(0), end_(0) {}
	void SetStart(int start);
	void SetEnd(int end);
	void SetActiveColor(const QColor& color);
	void highlightBlock(const QString& text) override;
};

