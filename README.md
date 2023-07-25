# ez-steam-api
This is a synchronous/blocking, flat, C-style wrapper around the [Steamworks SDK](https://partner.steamgames.com/doc/sdk), originally designed for use via foreign function interfaces.

This repository also includes a Node/JavaScript wrapper that is based on [Koffi](https://koffi.dev/). I used this wrapper for an Electron-based game on Steam (specifically for a Linux port).

## Motivation
The Steamworks API is native C++, with an optional flat/C-style API. Unfortunately, I found both provided options cumbersome in my project, for a few reasons:

* Callbacks are designed to be polled each frame--a hassle for event-driven environments
* Callbacks and call results are C++ member functions, with no out-of-the-box support for blocking (synchronous results) or promises
* The language I was using (JavaScript) doesn't have a good C++ interop story (whereas C interop is ubiquitous)

## Design
Internally, ez-steam-api spins up a thread for running Steam's callbacks and takes care of marshaling results back to calling threads (who just see a trivial synchronous API).

The synchronous, flat API can be trivially used with C-based foreign function interfaces that many languages provide. See the JavaScript wrapper in this repository for an example.

## Functionality and status
Currently, this library only exposes the functionality I needed for my game (listed below). Note: The code quality is "hobby ready" (as opposed to "production ready").

* Retrieve player's "persona name" (display name)
* Get, set, and persist achievements
* Set leaderboard scores
* Retrieve friends' leaderboard scores

Feel free to open an issue or pull request to add additional functionality.

## Usage
Here are some examples of using the API from JavaScript (via Koffi FFI) and C:

### JavaScript
```js
const { Steam } = require("ez-steam-api");

const appId = 12345; // Replace with your app id

const shouldExit = Steam.start(appId);
if (!shouldExit) {
    try {
        // Log the user's "persona" (display) name
        console.log(`Name: ${Steam.getUserName()}`);

        // Set an achievement (this assumes an achievement named "FOOBAR" exists for the app)
        const newlyAchieved = Steam.setAchievement("FOOBAR");
        if (newlyAchieved) {
            await Steam.storeAchievementsAsync();
        }

        // Get a friend leaderboard (this assumes a leaderboard named "Score" exists for the app)
        const leaderboard = await Steam.getLeaderboardAsync("Score");
        const json = await leaderboard.getFriendScoresAsync();

        for (const { name, score } of JSON.parse(json)) {
            console.log(`${name}: ${score}`);
        }
    } finally {
        Steam.stop();
    }
}
```

### C
```c
#include <stdlib.h>
#include <stdio.h>
#include "ez-steam-api.h"

int main(int argc, char** argv) {
    unsigned int app_id = 12345; // Replace with your app id
    int should_exit = 0;

    // Initialize Steam
    if (!ez_steam_start(app_id, &should_exit)) {
        return -1;
    }
    else if (should_exit) {
        return 0;
    }

    // Print the user's "persona" (display) name
    char* name = 0;
    if (ez_steam_user_name_get(&name)) {
        printf("Name: %s\n", name);
        ez_steam_string_free(name);
    }

    // Set an achievement (this assumes an achievement named "FOOBAR" exists for the app)
    int newly_achieved = 0;
    if (ez_steam_achievement_set("FOOBAR", &newly_achieved) && newly_achieved) {
        ez_steam_achivements_store();
    }

    // Get a friend leaderboard (this assumes a leaderboard named "Score" exists for the app)
    unsigned long long leaderboard = 0;
    if (ez_steam_leaderboard_get("Score", &leaderboard)) {
        char *json = 0;
        if (ez_steam_leaderboard_get_friend_scores(leaderboard, &json)) {
            printf("Leaderboard as JSON: %s\n", json);
            ez_steam_string_free(json);
        }
    }

    ez_steam_stop();
    return 0;
}
```

## Versioning and compatibility
The second (minor) version number corresponds to the Steamworks SDK's second version component. For example, X.**57**.Y is major version X, targeting Steamworks SDK 1.**57**, with patch version Y.
