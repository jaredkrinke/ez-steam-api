#pragma once

#include <vector>
#include <memory>
#include <functional>
#include <thread>
#include <mutex>
#include <condition_variable>

#include <steam/steam_api.h>
#include <steam/isteamuserstats.h>

class SteamCallManager;

// Helper for running a function on scope exit
class scope_exit {
public:
    scope_exit(std::function<void()> f)
        : m_f(f) {
    }

    ~scope_exit() {
        m_f();
    }

private:
    std::function<void()> m_f;
};

// Helper class for serializing a specific Steam API call that returns a Call Result
template<typename TSteamResult, typename TState, typename TResult, typename ...TArgs>
class SteamCall {
public:
    SteamCall(SteamCallManager* parent, std::function<SteamAPICall_t(TArgs...)> start, std::function<void(TSteamResult*, TState*)> translateResult)
        : m_parent(parent), m_start(start), m_translateResult(translateResult), m_state(nullptr) {
    }

    ~SteamCall() {
        // Note: This isn't meticulously synchronized and doesn't update the outstanding call count because
        // SteamAPI_RunCallbacks won't be called again in the caller anyway
        if (m_state != nullptr) {
            m_state->succeeded = false;
        }

        m_completed.notify_all();
    }

    // (Hopefully) Thread-safe, serialized, synchronous call to Steam API
    TResult Call(TArgs...args) {
        std::lock_guard<std::mutex> callbackLock(m_callLock);

        SteamAPICall_t call = m_start(args...);
        // TODO: Couldn't this be a member variable?
        auto state = std::make_unique<TState>();
        m_state = state.get();

        m_callResult.Set(call, this, &SteamCall<TSteamResult, TState, TResult, TArgs...>::OnCallback);
        m_parent->IncrementOutstandingCallCount();

        std::lock_guard<std::mutex> completedLock(m_completedLock);
        m_completed.wait(m_completedLock);

        if (!state->succeeded) {
            throw new std::runtime_error("SteamCall failed!");
        }

        return state->data;
    }

private:
    // Call result callback
    void OnCallback(TSteamResult* result, bool ioFailed) {
        auto onScopeExit = scope_exit([&] {
            m_parent->DecrementOutstandingCallCount();
            m_completed.notify_all();
        });

        if (ioFailed) {
            m_state->succeeded = false;
            return;
        }
        else {
            m_state->succeeded = true;
            m_translateResult(result, m_state);
        }
    }

    SteamCallManager* m_parent;

    // Functions for calling the Steam API and processing the result
    std::function<SteamAPICall_t(TArgs...)> m_start;
    std::function<void(TSteamResult*, TState*)> m_translateResult;

    // Synchronization and state
    std::mutex m_callLock;
    CCallResult<SteamCall<TSteamResult, TState, TResult, TArgs...>, TSteamResult> m_callResult;
    TState* m_state;
    std::mutex m_completedLock;
    std::condition_variable_any m_completed;
};

typedef struct {
    bool succeeded;
    SteamLeaderboard_t data;
} GetLeaderboardState;

typedef struct {
    bool succeeded;
    bool data;
} SetLeaderboardEntryState;

typedef struct {
    std::string name;
    int score;
} FriendLeaderboardRow;

typedef struct {
    bool succeeded;
    std::vector<FriendLeaderboardRow> data;
} GetFriendLeaderboardEntriesState;

class SteamCallManager {
public:
    const int pollingPeriodMS = 200;

    SteamCallManager(unsigned long long appId);
    ~SteamCallManager();

    // Called by SteamCall<...> helpers
    unsigned int GetOutstandingCallCount();
    void IncrementOutstandingCallCount();
    void DecrementOutstandingCallCount();

    // SteamCallManager thread function
    void RunThread();

    // Synchronous (serialized) calls
    SteamLeaderboard_t GetLeaderboard(const char* name);
    std::vector<FriendLeaderboardRow> GetFriendLeaderboardEntries(SteamLeaderboard_t nativeHandle);
    bool SetLeaderboardEntry(SteamLeaderboard_t nativeHandle, int score, const int* scoreDetails, int scoreDetailsCount);

    // Achievements
    bool GetAchievement(const char* achievementId);
    bool SetAchievement(const char* achievementId);
    void StoreAchievements();

    STEAM_CALLBACK(SteamCallManager, OnUserStatsReceived, UserStatsReceived_t, m_callbackUserStatsReceived);
    STEAM_CALLBACK(SteamCallManager, OnUserStatsStored, UserStatsStored_t, m_callbackUserStatsStored);
    STEAM_CALLBACK(SteamCallManager, OnAchievementStored, UserAchievementStored_t, m_callbackAchievementStored);

private:
    unsigned long long m_appId;

    std::unique_ptr<std::thread> m_thread;

    std::atomic_int m_outstandingCalls;
    std::mutex m_startOrShutdownLock;
    bool m_shouldShutdown;
    std::condition_variable_any m_startOrShutdown;

    bool m_achievementsInitialized;

    SteamCall<LeaderboardFindResult_t, GetLeaderboardState, SteamLeaderboard_t, const char*> m_getLeaderboard;
    SteamCall<LeaderboardScoresDownloaded_t, GetFriendLeaderboardEntriesState, std::vector<FriendLeaderboardRow>, SteamLeaderboard_t> m_getFriendLeaderboardEntries;
    SteamCall<LeaderboardScoreUploaded_t, SetLeaderboardEntryState, bool, SteamLeaderboard_t, int, const int*, int> m_setLeaderboardEntry;
};
