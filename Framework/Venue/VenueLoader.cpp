// VenueLoader.cpp

#include "VenueManager.h"

namespace NiuMa {
	VenueLoader::VenueLoader(int gameType)
		: _gameType(gameType)
	{}

	VenueLoader::~VenueLoader() {}

	int VenueLoader::getGameType() const {
		return _gameType;
	}
}