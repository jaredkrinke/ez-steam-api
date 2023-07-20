#ifndef PCH_H
#define PCH_H

// Windows-specific headers
#ifdef _WIN32
#define _CRT_SECURE_NO_WARNINGS
#endif

// C
#include <stdlib.h>

// C++
#include <string>
#include <stdexcept>
#include <memory>
#include <vector>
#include <functional>
#include <thread>
#include <mutex>
#include <condition_variable>

// Steam
#include <steam/steam_api.h>
#include <steam/isteamfriends.h>
#include <steam/isteamuserstats.h>

// Other
#include "../deps/json/nlohmann/json.hpp"

// Local
#include "steam-call-manager.h"

#endif
