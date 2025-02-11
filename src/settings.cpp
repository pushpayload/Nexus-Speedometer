#include "settings.h"

#include "shared.h"

#include <filesystem>
#include <fstream>

const char* IS_HW_VISIBLE = "IsHWVisible";
const char* IS_INDICATOR_ENABLED = "IsIndicatorEnabled";
const char* IS_INDICATOR_LOCKED = "IsIndicatorLocked";
const char* IS_INDICATOR_AUTO_RESIZE = "IsIndicatorAutoResize";

namespace Settings {
std::mutex Mutex;
json Settings = json::object();

void Load(std::filesystem::path aPath) {
    if (!std::filesystem::exists(aPath)) {
        return;
    }

    Settings::Mutex.lock();
    {
        std::ifstream file(aPath);
        auto result = json::parse(file, nullptr, false);
        if (result.is_discarded()) {
            APIDefs->Log(ELogLevel_WARNING, "Nexus-HelloWorld", "Settings.json could not be parsed.");
        } else {
            Settings = result;
        }
        file.close();
    }
    Settings::Mutex.unlock();

    if (!Settings[IS_INDICATOR_ENABLED].is_null()) {
        Settings[IS_INDICATOR_ENABLED].get_to<bool>(IsIndicatorEnabled);
    }
    if (!Settings[IS_INDICATOR_LOCKED].is_null()) {
        Settings[IS_INDICATOR_LOCKED].get_to<bool>(IsIndicatorLocked);
    }
    if (!Settings[IS_INDICATOR_AUTO_RESIZE].is_null()) {
        Settings[IS_INDICATOR_AUTO_RESIZE].get_to<bool>(IsIndicatorAutoResize);
    }
}

void Save(std::filesystem::path aPath) {
    Settings::Mutex.lock();
    {
        std::ofstream file(aPath);
        file << Settings.dump(1, '\t') << std::endl;
        file.close();
    }
    Settings::Mutex.unlock();
}

/* Global */

/* Indicator */
bool IsIndicatorEnabled = true;
bool IsIndicatorLocked = false;
bool IsIndicatorAutoResize = true;
} // namespace Settings
