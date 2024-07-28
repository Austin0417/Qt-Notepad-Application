#pragma once
#include <iostream>
#include "Connection.h"


enum class ServerToClientHeaders
{
	SEND_ID = 1,
	SEND_TEXT,
	SEND_CLIENT_CURSOR_POS,
	SEND_SELECTION_DATA,
	SERVER_REMOVE_SELECTION,
	SERVER_REMOVE_CHAR,
	SERVER_END = 9
};

