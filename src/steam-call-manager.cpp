#include "pch.h"

SteamCallManager::SteamCallManager(unsigned long long appId)
    : m_appId(appId),
    m_achievementsInitialized(false),
    m_callbackUserStatsReceived(this, &SteamCallManager::OnUserStatsReceived),
    m_callbackUserStatsStored(this, &SteamCallManager::OnUserStatsStored),
    m_callbackAchievementStored(this, &SteamCallManager::OnAchievementStored),
    m_getLeaderboard(this,
        [](const char* name) -> SteamAPICall_t {
            return SteamUserStats()->FindLeaderboard(name);
        },
        [](LeaderboardFindResult_t* result, GetLeaderboardState* state) -> void {
            if (!result->m_bLeaderboardFound) {
                state->succeeded = false;
            }
            else {
                state->data = result->m_hSteamLeaderboard;
            }
        }),
    m_getFriendLeaderboardEntries(this,
        [](SteamLeaderboard_t nativeHandle) -> SteamAPICall_t {
            return SteamUserStats()->DownloadLeaderboardEntries(nativeHandle, k_ELeaderboardDataRequestFriends, 0, 0);
        },
        [](LeaderboardScoresDownloaded_t* result, GetFriendLeaderboardEntriesState* state) -> void {
            auto stats = SteamUserStats();
            auto friends = SteamFriends();
            for (int i = 0; i < result->m_cEntryCount; i++) {
                LeaderboardEntry_t entry;
                if (!stats->GetDownloadedLeaderboardEntry(result->m_hSteamLeaderboardEntries, i, &entry, nullptr, 0)) {
                    throw new std::runtime_error("GetDownloadedLeaderboardEntry failed!");
                }

                state->data.push_back({ friends->GetFriendPersonaName(entry.m_steamIDUser), entry.m_nScore });
            }
        }),
    m_setLeaderboardEntry(this,
        [](SteamLeaderboard_t nativeHandle, int score, const int* scoreDetails, int scoreDetailsCount) -> SteamAPICall_t {
            return SteamUserStats()->UploadLeaderboardScore(nativeHandle, k_ELeaderboardUploadScoreMethodKeepBest, score, scoreDetails, scoreDetailsCount);
        },
        [](LeaderboardScoreUploaded_t* result, SetLeaderboardEntryState* state) -> void {
            if (result->m_bSuccess) {
                state->data = result->m_bScoreChanged;
            }
            else {
                state->succeeded = false;
            }
        })
{
    m_thread = std::make_unique<std::thread>([this]() {
        this->RunThread();
    });

    // Kick off user stats request (to initialize achievements)
    if (SteamUserStats()->RequestCurrentStats()) {
        // Should get a OnUserStatsReceived callback
        IncrementOutstandingCallCount();
    }
}

SteamCallManager::~SteamCallManager() {
    {
        std::lock_guard<std::mutex> lock(m_startOrShutdownLock);
        m_shouldShutdown = true;
        m_startOrShutdown.notify_all();
    }

    m_thread->join();
}

unsigned int SteamCallManager::GetOutstandingCallCount() {
    return m_outstandingCalls.load();
}

void SteamCallManager::IncrementOutstandingCallCount() {
    if (++m_outstandingCalls == 1) {
        // First outstanding call; kick off processing now
        m_startOrShutdown.notify_all();
    }
}

void SteamCallManager::DecrementOutstandingCallCount() {
    --m_outstandingCalls;
}

void SteamCallManager::RunThread() {
    while (true) {
        std::lock_guard<std::mutex> lock(m_startOrShutdownLock);
        m_startOrShutdown.wait(m_startOrShutdownLock);

        if (m_shouldShutdown) {
            // Shutdown
            break;
        }
        else {
            // Start processing
            // TODO: Could lock be released first?
            while (true) {
                SteamAPI_RunCallbacks();
                if (GetOutstandingCallCount() <= 0) {
                    break;
                }

                std::this_thread::sleep_for(std::chrono::milliseconds(SteamCallManager::pollingPeriodMS));
            }
        }
    }
}

SteamLeaderboard_t SteamCallManager::GetLeaderboard(const char* name) {
    return m_getLeaderboard.Call(name);
}

std::vector<FriendLeaderboardRow> SteamCallManager::GetFriendLeaderboardEntries(SteamLeaderboard_t nativeHandle) {
    return m_getFriendLeaderboardEntries.Call(nativeHandle);
}

bool SteamCallManager::SetLeaderboardEntry(SteamLeaderboard_t nativeHandle, int score, const int* scoreDetails, int scoreDetailsCount) {
    return m_setLeaderboardEntry.Call(nativeHandle, score, scoreDetails, scoreDetailsCount);
}

bool SteamCallManager::GetAchievement(const char* achievementId) {
    auto userStats = SteamUserStats();
    bool achieved = false;

    if (!userStats) {
        throw new std::runtime_error("SteamUserStats returned null!");
    }

    if (!userStats->GetAchievement(achievementId, &achieved)) {
        throw new std::runtime_error("GetAchievement failed!");
    }

    return achieved;
}

bool SteamCallManager::SetAchievement(const char* achievementId) {
    auto userStats = SteamUserStats();
    bool achieved = false;

    if (!userStats) {
        throw new std::runtime_error("SteamUserStats returned null!");
    }

    if (!userStats->GetAchievement(achievementId, &achieved)) {
        throw new std::runtime_error("GetAchievement failed!");
    }

    if (achieved) {
        return false;
    }
    else {
        if (!userStats->SetAchievement(achievementId)) {
            throw new std::runtime_error("SetAchievement failed!");
        }

        return true;

        // Make sure to call StoreAchievements eventually!
    }
}

void SteamCallManager::StoreAchievements() {
    auto userStats = SteamUserStats();

    if (!userStats) {
        throw new std::runtime_error("SteamUserStats returned null!");
    }

    if (!userStats->StoreStats()) {
        throw new std::runtime_error("StoreStats failed!");
    }

    // Should get OnUserStatsStored callback. OnAchievementStored is only hit if the achievement was newly achieved.
    IncrementOutstandingCallCount();
}

void SteamCallManager::OnUserStatsReceived(UserStatsReceived_t* data) {
    if (!m_achievementsInitialized && data->m_nGameID == m_appId) {
        DecrementOutstandingCallCount();
        if (data->m_eResult == k_EResultOK) {
            m_achievementsInitialized = true;
        }
    }

}

void SteamCallManager::OnUserStatsStored(UserStatsStored_t* data) {
    if (data->m_nGameID == m_appId) {
        DecrementOutstandingCallCount();
    }
}

void SteamCallManager::OnAchievementStored(UserAchievementStored_t* data) {
}
