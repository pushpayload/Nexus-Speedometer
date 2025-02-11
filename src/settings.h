#pragma once
#ifndef SETTINGS_H
    #define SETTINGS_H

    #include <filesystem>
    #include <mutex>

    #include "nlohmann/json.hpp"
    #include "shared.h"

using json = nlohmann::json;

extern const char* IS_SPEED_VISIBLE;
extern const char* IS_INDICATOR_ENABLED;
extern const char* IS_INDICATOR_LOCKED;
extern const char* IS_INDICATOR_AUTO_RESIZE;
extern const char* IS_INDICATOR_TRANSPARENT;
extern const char* SPEED_FORMAT;
extern const char* SHOW_CURRENT_SPEED;
extern const char* SHOW_MAX_SPEED;
extern const char* SHOW_CUSTOM_SPEED;
extern const char* LOG_LEVEL;

namespace Settings {
extern std::mutex Mutex;
extern json Settings;

/* Loads the settings. */
void Load(std::filesystem::path aPath);
/* Saves the settings. */
void Save(std::filesystem::path aPath);

/* Global */

/* Indicator */
extern bool IsIndicatorEnabled;
extern bool IsIndicatorLocked;
extern bool IsIndicatorAutoResize;
extern bool IsIndicatorTransparent;
extern std::string SpeedFormat;
extern bool ShowCurrentSpeed;
extern bool ShowMaxSpeed;
extern bool ShowCustomSpeed;

/* Log Level */
extern ELogLevel LogLevel;

} // namespace Settings

#endif
