#include "settings.h"

#include "shared.h"

#include <filesystem>
#include <fstream>

const char* IS_SPEED_VISIBLE = "IsSpeedVisible";
const char* IS_INDICATOR_ENABLED = "IsIndicatorEnabled";
const char* IS_INDICATOR_LOCKED = "IsIndicatorLocked";
const char* IS_INDICATOR_AUTO_RESIZE = "IsIndicatorAutoResize";
const char* IS_INDICATOR_TRANSPARENT = "IsIndicatorTransparent";
const char* SPEED_FORMAT = "SpeedFormat";
const char* SHOW_CURRENT_SPEED = "ShowCurrentSpeed";
const char* SHOW_MAX_SPEED = "ShowMaxSpeed";
const char* SHOW_CUSTOM_SPEED = "ShowCustomSpeed";
const char* LOG_LEVEL = "LogLevel";
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
    if (!Settings[IS_INDICATOR_TRANSPARENT].is_null()) {
        Settings[IS_INDICATOR_TRANSPARENT].get_to<bool>(IsIndicatorTransparent);
    }
    if (!Settings[SPEED_FORMAT].is_null()) {
        Settings[SPEED_FORMAT].get_to<std::string>(SpeedFormat);
    }
    if (!Settings[SHOW_CURRENT_SPEED].is_null()) {
        Settings[SHOW_CURRENT_SPEED].get_to<bool>(ShowCurrentSpeed);
    }
    if (!Settings[SHOW_MAX_SPEED].is_null()) {
        Settings[SHOW_MAX_SPEED].get_to<bool>(ShowMaxSpeed);
    }
    if (!Settings[SHOW_CUSTOM_SPEED].is_null()) {
        Settings[SHOW_CUSTOM_SPEED].get_to<bool>(ShowCustomSpeed);
    }
    if (!Settings[LOG_LEVEL].is_null()) {
        Settings[LOG_LEVEL].get_to<int>(reinterpret_cast<int&>(LogLevel));
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
bool IsIndicatorTransparent = false;
std::string SpeedFormat = "c: {current:.2f} | m: {max:2f}";
bool ShowCurrentSpeed = true;
bool ShowMaxSpeed = false;
bool ShowCustomSpeed = false;

/* Log Level */
ELogLevel LogLevel = ELogLevel::ELogLevel_INFO;
} // namespace Settings
