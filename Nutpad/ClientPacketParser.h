#pragma once
#include <iostream>
#include "Connection.h"
#include "NetworkStructures.h"
#include "ColorHelper.h"


ClientColorPacket GetClientColorPacketFromStream(std::istream& is);

std::vector<ClientColorPacket> GetAllClientColorsFromStream(std::istream& is);

ClientCursorPositionData GetCursorPositionDataFromStream(std::istream& is);

ClientRemovedCharacterData GetCharRemovedDataFromStream(std::istream& is);

ClientSelectionData GetClientSelectionDataFromStream(std::istream& is);

ClientRemovedSelectionData GetClientRemovedSelectionDataFromStream(std::istream& is);

std::ostream& operator<<(std::ostream& os, const ClientCursorPositionData& data);

std::ostream& operator<<(std::ostream& os, const ClientSelectionData& data);

std::ostream& operator<<(std::ostream& os, const ClientRemovedCharacterData& data);

std::ostream& operator<<(std::ostream& os, const ClientRemovedSelectionData& data);