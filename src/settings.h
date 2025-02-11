#pragma once
#ifndef SETTINGS_H
    #define SETTINGS_H

    #include <filesystem>
    #include <mutex>

    #include "nlohmann/json.hpp"
using json = nlohmann::json;

extern const char* IS_HW_VISIBLE;
extern const char* IS_INDICATOR_ENABLED;
extern const char* IS_INDICATOR_LOCKED;
extern const char* IS_INDICATOR_AUTO_RESIZE;

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

} // namespace Settings

#endif
