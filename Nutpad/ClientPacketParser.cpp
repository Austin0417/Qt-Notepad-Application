#include "ClientPacketParser.h"


ClientCursorPositionData GetCursorPositionDataFromStream(std::istream& is)
{
	char client_id_char_buffer[CursorPositionData::NUM_DIGITS_CLIENT_ID_ + 1];
	is.read(client_id_char_buffer, CursorPositionData::NUM_DIGITS_CLIENT_ID_);
	client_id_char_buffer[CursorPositionData::NUM_DIGITS_CLIENT_ID_] = 0;

	char cursor_index_char_buffer[CursorPositionData::NUM_DIGITS_CURSOR_INDEX_ + 1];
	is.read(cursor_index_char_buffer, CursorPositionData::NUM_DIGITS_CURSOR_INDEX_);
	cursor_index_char_buffer[CursorPositionData::NUM_DIGITS_CURSOR_INDEX_] = 0;

	return ClientCursorPositionData(std::atoi(client_id_char_buffer), std::atoi(cursor_index_char_buffer));
}

ClientRemovedCharacterData GetCharRemovedDataFromStream(std::istream& is)
{
	char client_id_buffer[CharacterRemovedData::NUM_DIGITS_CLIENT_ID_ + 1];
	is.read(client_id_buffer, CharacterRemovedData::NUM_DIGITS_CLIENT_ID_);
	client_id_buffer[CharacterRemovedData::NUM_DIGITS_CLIENT_ID_] = 0;

	char index_buffer[CharacterRemovedData::NUM_DIGITS_INDEX_CHAR_REMOVED + 1];
	is.read(index_buffer, CharacterRemovedData::NUM_DIGITS_INDEX_CHAR_REMOVED);
	index_buffer[CharacterRemovedData::NUM_DIGITS_INDEX_CHAR_REMOVED] = 0;

	return ClientRemovedCharacterData(std::atoi(client_id_buffer), std::atoi(index_buffer));
}

ClientSelectionData GetClientSelectionDataFromStream(std::istream& is)
{
	int client_id;
	int selection_start;
	int selection_end;

	char client_id_buffer[SelectionData::NUM_DIGITS_CLIENT_ID + 1];
	is.read(client_id_buffer, SelectionData::NUM_DIGITS_CLIENT_ID);
	client_id_buffer[SelectionData::NUM_DIGITS_CLIENT_ID] = 0;

	char selection_start_buffer[SelectionData::NUM_DIGITS_START_INDEX + 1];
	is.read(selection_start_buffer, SelectionData::NUM_DIGITS_START_INDEX);
	selection_start_buffer[SelectionData::NUM_DIGITS_START_INDEX] = 0;

	char selection_end_buffer[SelectionData::NUM_DIGITS_END_INDEX + 1];
	is.read(selection_end_buffer, SelectionData::NUM_DIGITS_END_INDEX);
	selection_end_buffer[SelectionData::NUM_DIGITS_END_INDEX] = 0;

	client_id = std::atoi(client_id_buffer);
	selection_start = std::atoi(selection_start_buffer);
	selection_end = std::atoi(selection_end_buffer);

	return ClientSelectionData{ client_id, selection_start, selection_end };
}

std::ostream& operator<<(std::ostream& os, const ClientCursorPositionData& data)
{
	int num_digits_client_id = GetNumDigitsFromInteger(data.client_id_);
	for (int i = num_digits_client_id; i < CursorPositionData::NUM_DIGITS_CLIENT_ID_; i++)
	{
		os << 0;
	}
	os << data.client_id_;

	int num_digits_cursor_index = GetNumDigitsFromInteger(data.cursor_position_index_);
	for (int i = num_digits_cursor_index; i < CursorPositionData::NUM_DIGITS_CURSOR_INDEX_; i++)
	{
		os << 0;
	}
	os << data.cursor_position_index_;
	return os;
}

std::ostream& operator<<(std::ostream& os, const ClientSelectionData& data)
{
	int num_digits_client_id = GetNumDigitsFromInteger(data.client_id_);
	for (int i = num_digits_client_id; i < SelectionData::NUM_DIGITS_CLIENT_ID; i++)
	{
		os << 0;
	}
	os << data.client_id_;

	int digits_start_index = GetNumDigitsFromInteger(data.start_);
	int digits_end_index = GetNumDigitsFromInteger(data.end_);

	for (int i = digits_start_index; i < SelectionData::NUM_DIGITS_START_INDEX; i++)
	{
		os << 0;
	}
	os << data.start_;

	for (int i = digits_end_index; i < SelectionData::NUM_DIGITS_END_INDEX; i++)
	{
		os << 0;
	}
	os << data.end_;
	return os;
}

std::ostream& operator<<(std::ostream& os, const ClientRemovedCharacterData& data)
{
	int num_digits_client_id = GetNumDigitsFromInteger(data.client_id_);
	for (int i = num_digits_client_id; i < CharacterRemovedData::NUM_DIGITS_CLIENT_ID_; i++)
	{
		os << 0;
	}
	os << data.client_id_;

	int num_digits_char_index = GetNumDigitsFromInteger(data.char_index_to_remove_);
	for (int i = num_digits_char_index; i < CharacterRemovedData::NUM_DIGITS_INDEX_CHAR_REMOVED; i++)
	{
		os << 0;
	}
	os << data.char_index_to_remove_;
	return os;
}
