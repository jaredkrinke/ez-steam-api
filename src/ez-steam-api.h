#pragma once

// Definitions for C vs. C++, Windows, etc.
#ifdef __cplusplus
#define EZ_STEAM_DEF_PREFIX extern "C"
#define EZ_STEAM_DEF_SUFFIX noexcept
#endif

#ifdef _WIN32
#ifdef EZ_STEAM_IMPLEMENTATION
#define EZ_STEAM_DEF_ATTRIBUTES __declspec(dllexport)
#else
#define EZ_STEAM_DEF_ATTRIBUTES __declspec(dllimport)
#endif
#else
#define EZ_STEAM_DEF_ATTRIBUTES 
#endif

#define EZ_STEAM_DEF EZ_STEAM_DEF_PREFIX EZ_STEAM_DEF_ATTRIBUTES int

// Startup and shutdown
EZ_STEAM_DEF ez_steam_start(unsigned int app_id, int* out_should_restart) EZ_STEAM_DEF_SUFFIX;
EZ_STEAM_DEF ez_steam_stop() EZ_STEAM_DEF_SUFFIX;

// Memory handling
EZ_STEAM_DEF ez_steam_string_free(char* data) EZ_STEAM_DEF_SUFFIX;

// User info
EZ_STEAM_DEF ez_steam_user_name_get(char** out_user_name) EZ_STEAM_DEF_SUFFIX;

// Language settings
EZ_STEAM_DEF ez_steam_app_language_get(char** out_language) EZ_STEAM_DEF_SUFFIX;

// Leaderboards
EZ_STEAM_DEF ez_steam_leaderboard_get(const char* leaderboard_name, unsigned long long* out_leaderboard) EZ_STEAM_DEF_SUFFIX;
EZ_STEAM_DEF ez_steam_leaderboard_set_score(unsigned long long leaderboard, int score, const int* detail, int detail_count, int* out_score_updated) EZ_STEAM_DEF_SUFFIX;
EZ_STEAM_DEF ez_steam_leaderboard_get_friend_scores(unsigned long long leaderboard, char** friend_scores_json) EZ_STEAM_DEF_SUFFIX;

// Achievements
EZ_STEAM_DEF ez_steam_achievement_get(const char* achievement_id, int* achieved) EZ_STEAM_DEF_SUFFIX;
EZ_STEAM_DEF ez_steam_achievement_set(const char* achievement_id, int* newly_achieved) EZ_STEAM_DEF_SUFFIX;
EZ_STEAM_DEF ez_steam_achivements_store() EZ_STEAM_DEF_SUFFIX;
