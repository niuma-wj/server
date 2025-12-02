// RedisKeys.cpp

#include "RedisKeys.h"

namespace NiuMa {
	RedisKeys::RedisKeys() {}

	RedisKeys::~RedisKeys() {}

	const std::string RedisKeys::PLAYER_MESSAGE_SECRET("player_message_secret:");

	const std::string RedisKeys::PLAYER_CURRENT_VENUE("player_current_venue:");

	const std::string RedisKeys::PLAYER_AUTHORIZED_VENUE("player_authorized_venue:");

	const std::string RedisKeys::IP_BLACKLIST("ip_blacklist");

	const std::string RedisKeys::SERVER_PLAYER_COUNT("server_player_count:");

	const std::string RedisKeys::VENUE_SERVER_MAP("venue_server_map:");

	const std::string RedisKeys::VENUE_SERVER_SET("venue_server_set");

	const std::string RedisKeys::VENUE_PLAYER_COUNT("venue_player_count:");

	const std::string RedisKeys::SERVER_ACCESS_ADDRESS("server_access_address:");

	const std::string RedisKeys::SERVER_WS_ADDRESS("server_ws_address:");

	const std::string RedisKeys::SERVER_KEEP_ALIVE("server_keep_alive:");

	const std::string RedisKeys::DISTRICT_VENUE_REGISTER("district_venue_register_");

	const std::string RedisKeys::DISTRICT_NOT_FULL_VENUES("district_not_full_venues_");

	const std::string RedisKeys::DISTRICT_PLAYER_TRACK("district_{0}_player_track:{1}");
}