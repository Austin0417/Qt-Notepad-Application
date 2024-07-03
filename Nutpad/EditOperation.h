#pragma once
#include <QString>


class EditOperation
{
public:
	enum class EditType
	{
		ADD,
		ERASE
	};

private:
	EditType edit_type_;
	int index_of_edit_;
	QString edit_content_;

public:
	EditOperation();
	void SetEditType(EditType edit_type);
	void SetIndexOfEdit(int index);
	void SetEditContent(const QString& edit_content);
	void AddCharacterToEditContent(QChar additional_char);
	EditType GetEditType() const;
	int GetIndexOfEdit() const;
	const QString& GetEditContent() const;
	QString& GetEditContent();
	void ResetEditOperation();
};

