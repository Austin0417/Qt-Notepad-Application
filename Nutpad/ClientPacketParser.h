#pragma once
#include <iostream>
#include "Connection.h"
#include "NetworkStructures.h"




ClientCursorPositionData GetCursorPositionDataFromStream(std::istream& is);

ClientRemovedCharacterData GetCharRemovedDataFromStream(std::istream& is);

ClientSelectionData GetClientSelectionDataFromStream(std::istream& is);

std::ostream& operator<<(std::ostream& os, const ClientCursorPositionData& data);

std::ostream& operator<<(std::ostream& os, const ClientSelectionData& data);

std::ostream& operator<<(std::ostream& os, const ClientRemovedCharacterData& data);