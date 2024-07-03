#include "EditOperation.h"

EditOperation::EditOperation() :
	edit_type_(EditType::ADD),
	index_of_edit_(0)
{

}

void EditOperation::SetEditType(EditType edit_type)
{
	edit_type_ = edit_type;
}

void EditOperation::SetIndexOfEdit(int index)
{
	index_of_edit_ = index;
}

void EditOperation::SetEditContent(const QString& edit_content)
{
	edit_content_ = edit_content;
}

void EditOperation::AddCharacterToEditContent(QChar additional_char)
{
	edit_content_ += additional_char;
}

EditOperation::EditType EditOperation::GetEditType() const
{
	return edit_type_;
}

int EditOperation::GetIndexOfEdit() const
{
	return index_of_edit_;
}

const QString& EditOperation::GetEditContent() const
{
	return edit_content_;
}

QString& EditOperation::GetEditContent()
{
	return edit_content_;
}


void EditOperation::ResetEditOperation()
{
	edit_content_.clear();
}




