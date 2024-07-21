#pragma once

// When sending a structure over the network, the first character/byte will always be a header indicating the type of data the message holds
// Next two characters will hold id of the client from which the message originated from


struct ClientCursorPositionData
{
	ClientCursorPositionData(int client_id, int cursor_index) : client_id_(client_id), cursor_position_index_(cursor_index) {}
	int client_id_;
	int cursor_position_index_;
};

struct ClientRemovedCharacterData
{
	ClientRemovedCharacterData(int client_id, int index) : client_id_(client_id), char_index_to_remove_(index) {}
	int client_id_;
	int char_index_to_remove_;
};

struct ClientSelectionData
{
	int client_id_;
	int start_;
	int end_;

	ClientSelectionData(int id, int start, int end) : client_id_(id), start_(start), end_(end) {}
};

struct ClientRemovedSelectionData
{
	ClientRemovedSelectionData(int client_id, int start, int end) : client_id_(client_id), start_index_(start), end_index_(end) {}
	int client_id_;
	int start_index_;
	int end_index_;
};