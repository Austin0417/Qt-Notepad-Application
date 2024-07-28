#pragma once
#include <QSyntaxHighlighter>
#include <QTextEdit>
#include <functional>
#include "ClientTextData.h"


class NutpadTextHighlighter : public QSyntaxHighlighter
{
private:
	int start_;
	int end_;
	QColor current_active_color_;

	std::function<const std::unordered_map<int, ClientTextData>& ()> request_client_data_;
public:
	NutpadTextHighlighter(QTextDocument* text_document, const std::function<const std::unordered_map<int, ClientTextData>& ()>& request_client_data) : QSyntaxHighlighter(text_document), start_(0), end_(0), request_client_data_(request_client_data) {}
	NutpadTextHighlighter(QTextEdit* text_edit, const std::function<const std::unordered_map<int, ClientTextData>& ()>& request_client_data) : QSyntaxHighlighter(text_edit), start_(0), end_(0), request_client_data_(request_client_data) {}
	void SetStart(int start);
	void SetEnd(int end);
	void SetActiveColor(const QColor& color);
	void highlightBlock(const QString& text) override;
};

