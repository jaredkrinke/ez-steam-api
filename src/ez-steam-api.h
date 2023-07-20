#pragma once

#ifdef _WIN32
#define EZ_STEAM_DEF_ATTRIBUTES __declspec(dllimport)
#else
#define EZ_STEAM_DEF_ATTRIBUTES 
#endif

#define EZ_STEAM_DEF extern "C" EZ_STEAM_DEF_ATTRIBUTES int

// Startup and shutdown
EZ_STEAM_DEF ez_steam_start(unsigned int app_id, int* out_should_restart);
EZ_STEAM_DEF ez_steam_stop();

// Memory handling
EZ_STEAM_DEF ez_steam_string_free(char* data);

// User info
EZ_STEAM_DEF ez_steam_user_name_get(char** out_user_name);

// Leaderboards
EZ_STEAM_DEF ez_steam_leaderboard_get(const char* leaderboard_name, unsigned long long* out_leaderboard);
EZ_STEAM_DEF ez_steam_leaderboard_set_score(unsigned long long leaderboard, int score, const int* detail, int detail_count, int* out_score_updated);
EZ_STEAM_DEF ez_steam_leaderboard_get_friend_scores(unsigned long long leaderboard, char** friend_scores_json);

// Achievements
EZ_STEAM_DEF ez_steam_achievement_get(const char* achievement_id, int* achieved);
EZ_STEAM_DEF ez_steam_achievement_set(const char* achievement_id, int* newly_achieved);
EZ_STEAM_DEF ez_steam_achivements_store();
