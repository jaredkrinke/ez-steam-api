#include "pch.h"

#ifdef _WIN32
#define strdup _strdup
#endif

constexpr int EZ_STEAM_SUCCESS = 1;
constexpr int EZ_STEAM_ERROR = 0;

constexpr int EZ_STEAM_TRUE = 1;
constexpr int EZ_STEAM_FALSE = 0;

#define EZ_STEAM_CATCH catch (...) { return EZ_STEAM_ERROR; }
#define EZ_STEAM_BOOLIFY(a) (a ? EZ_STEAM_TRUE : EZ_STEAM_FALSE)

#define CHECK(a) if (!a) throw std::runtime_error("Steam API error!");

SteamCallManager* g_steam_call_manager = nullptr;

// Startup and shutdown
EZ_STEAM_DEF ez_steam_start(unsigned int app_id, int* out_should_restart) noexcept try {
	*out_should_restart = EZ_STEAM_FALSE;

	if (SteamAPI_RestartAppIfNecessary(app_id)) {
		*out_should_restart = EZ_STEAM_TRUE;
		return EZ_STEAM_SUCCESS;
	}

	CHECK(SteamAPI_Init());

	g_steam_call_manager = new SteamCallManager(app_id);

	return EZ_STEAM_SUCCESS;
}
EZ_STEAM_CATCH

EZ_STEAM_DEF ez_steam_stop() noexcept {
	if (g_steam_call_manager) {
		delete g_steam_call_manager;
	}

	SteamAPI_Shutdown();
	return EZ_STEAM_SUCCESS;
}

// Memory handling
char* duplicate_string(const std::string& str) {
	char* result = strdup(str.c_str());
	CHECK(result);
	return result;
}

EZ_STEAM_DEF ez_steam_string_free(char* data) noexcept {
	if (data) {
		free(data);
	}

	return EZ_STEAM_SUCCESS;
}

// User info
EZ_STEAM_DEF ez_steam_user_name_get(char** out_user_name) noexcept try {
	*out_user_name = nullptr;

	ISteamFriends* friends = SteamFriends();
	CHECK(friends);

	std::string user_name(friends->GetPersonaName());
	*out_user_name = duplicate_string(user_name);

	return EZ_STEAM_SUCCESS;
}
EZ_STEAM_CATCH

// Leaderboards
EZ_STEAM_DEF ez_steam_leaderboard_get(const char* leaderboard_name, unsigned long long* out_leaderboard) noexcept try {
	*out_leaderboard = 0;

	CHECK(g_steam_call_manager);
	*out_leaderboard = g_steam_call_manager->GetLeaderboard(leaderboard_name);

	return EZ_STEAM_SUCCESS;
}
EZ_STEAM_CATCH

EZ_STEAM_DEF ez_steam_leaderboard_set_score(unsigned long long leaderboard, int score, const int* detail, int detail_count, int* out_score_updated) noexcept try {
	*out_score_updated = EZ_STEAM_FALSE;

	CHECK(g_steam_call_manager);
	*out_score_updated = EZ_STEAM_BOOLIFY(g_steam_call_manager->SetLeaderboardEntry(leaderboard, score, detail, detail_count));

	return EZ_STEAM_SUCCESS;
}
EZ_STEAM_CATCH

EZ_STEAM_DEF ez_steam_leaderboard_get_friend_scores(unsigned long long leaderboard, char** friend_scores_json) noexcept try {
	*friend_scores_json = nullptr;

	CHECK(g_steam_call_manager);
	std::vector<FriendLeaderboardRow> rows = g_steam_call_manager->GetFriendLeaderboardEntries(leaderboard);

	nlohmann::json json;
	for (const auto& row : rows) {
		json.push_back({
			{ "name", row.name },
			{ "score", row.score },
			});
	}

	*friend_scores_json = duplicate_string(nlohmann::to_string(json).c_str());

	return EZ_STEAM_SUCCESS;
}
EZ_STEAM_CATCH

// Achievements
EZ_STEAM_DEF ez_steam_achievement_get(const char* achievement_id, int* achieved) noexcept try {
	*achieved = EZ_STEAM_FALSE;

	CHECK(g_steam_call_manager);
	*achieved = EZ_STEAM_BOOLIFY(g_steam_call_manager->GetAchievement(achievement_id));

	return EZ_STEAM_SUCCESS;
}
EZ_STEAM_CATCH

EZ_STEAM_DEF ez_steam_achievement_set(const char* achievement_id, int* newly_achieved) noexcept try {
	*newly_achieved = EZ_STEAM_FALSE;

	CHECK(g_steam_call_manager);
	*newly_achieved = EZ_STEAM_BOOLIFY(g_steam_call_manager->SetAchievement(achievement_id));

	return EZ_STEAM_SUCCESS;
}
EZ_STEAM_CATCH

EZ_STEAM_DEF ez_steam_achivements_store() noexcept try {
	CHECK(g_steam_call_manager);
	g_steam_call_manager->StoreAchievements();

	return EZ_STEAM_SUCCESS;
}
EZ_STEAM_CATCH
