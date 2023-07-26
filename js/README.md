# ez-steam-api
This is a Node+JavaScript wrapper around the [Steamworks SDK](https://partner.steamgames.com/doc/sdk) (based on [Koffi](https://koffi.dev/)), originally created for use in an Electron-based game on Steam.

## Motivation
I opted to create my own wrapper instead of using [Greenworks](https://github.com/greenheartgames/greenworks) for a couple of reasons:

* Greenworks is no longer maintained
* Greenworks doesn't expose "friend leaderboard" functionality
* I wanted a wrapper that could be reused easily from *any* language (with a foreign function interface-friendly, synchronous C API)

## Usage
Currently, this library only exposes the functionality I needed for my game. Example code:

```js
const { Steam } = require("ez-steam-api");

const appId = 12345; // Replace with your app id

(async () => {
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
            const rows = await leaderboard.getFriendScoresAsync();
    
            console.log("Leaderboard:");
            for (const { name, score } of rows) {
                console.log(`${name}: ${score}`);
            }
        } finally {
            Steam.stop();
        }
    }
})();
```

## Versioning and compatibility
The second (minor) version number corresponds to the Steamworks SDK's second version component. For example, X.**57**.Y is major version X, targeting Steamworks SDK 1.**57**, with patch version Y.
