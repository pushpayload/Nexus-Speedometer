#include "settings.h"
#include "shared.h"
#include "speedreader_loader.h"

#include "version.h"

void ProcessKeybind(const char* aIdentifier, bool aIsRelease);
void OnWindowResized(void* aEventArgs);
void OnMumbleIdentityUpdated(void* aEventArgs);
void ReceiveTexture(const char* aIdentifier, Texture* aTexture);

void AddonLoad(AddonAPI* aApi);
void AddonUnload();
void AddonRender();
void AddonOptions();
void AddonShortcut();
void AddonLog(const std::string& message, ELogLevel level = ELogLevel_INFO);

HMODULE hSelf;

AddonDefinition AddonDef{};

std::filesystem::path AddonPath;
std::filesystem::path SettingsPath;

const char* SPEED_TOGGLEVIS = "KB_SPEED_TOGGLEVIS";
const char* WINDOW_RESIZED = "EV_WINDOW_RESIZED";
const char* MUMBLE_IDENTITY_UPDATED = "EV_MUMBLE_IDENTITY_UPDATED";

SpeedReaderLoader g_speedReader;

const char* LOG_LEVEL_NAMES[] = {"Off", "Critical", "Warning", "Info", "Debug", "Trace", "All"};
const int LOG_LEVEL_COUNT = IM_ARRAYSIZE(LOG_LEVEL_NAMES);

std::string FormatSpeed(const std::string& format, float current, float max) {
    try {
        std::string result = format;

        // Convert \n to actual newlines
        size_t pos = 0;
        while ((pos = result.find("\\n", pos)) != std::string::npos) {
            result.replace(pos, 2, "\n");
            pos += 1;
        }

        // Replace {current} placeholder
        while ((pos = result.find("{current")) != std::string::npos) {
            size_t endPos = result.find("}", pos);
            if (endPos == std::string::npos) break;

            std::string placeholder = result.substr(pos, endPos - pos + 1);
            int precision = 1; // default precision

            try {
                // Check for custom precision
                size_t precPos = placeholder.find(":");
                if (precPos != std::string::npos) {
                    size_t dotPos = placeholder.find(".", precPos);
                    if (dotPos != std::string::npos && dotPos + 1 < placeholder.length()) {
                        precision = std::clamp(std::stoi(placeholder.substr(dotPos + 1, 1)), 0, 9);
                    }
                }
            } catch (...) {
                precision = 1; // Reset to default on any error
            }

            char formatted[32];
            snprintf(formatted, sizeof(formatted), "%.*f", precision, current);
            result.replace(pos, endPos - pos + 1, formatted);
        }

        // Replace {max} placeholder
        while ((pos = result.find("{max")) != std::string::npos) {
            size_t endPos = result.find("}", pos);
            if (endPos == std::string::npos) break;

            std::string placeholder = result.substr(pos, endPos - pos + 1);
            int precision = 1; // default precision

            try {
                // Check for custom precision
                size_t precPos = placeholder.find(":");
                if (precPos != std::string::npos) {
                    size_t dotPos = placeholder.find(".", precPos);
                    if (dotPos != std::string::npos && dotPos + 1 < placeholder.length()) {
                        precision = std::clamp(std::stoi(placeholder.substr(dotPos + 1, 1)), 0, 9);
                    }
                }
            } catch (...) {
                precision = 1; // Reset to default on any error
            }

            char formatted[32];
            snprintf(formatted, sizeof(formatted), "%.*f", precision, max);
            result.replace(pos, endPos - pos + 1, formatted);
        }

        return result;
    } catch (...) {
        return "Format Error";
    }
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH: hSelf = hModule; break;
    case DLL_PROCESS_DETACH: break;
    case DLL_THREAD_ATTACH: break;
    case DLL_THREAD_DETACH: break;
    }
    return TRUE;
}

extern "C" __declspec(dllexport) AddonDefinition* GetAddonDef() {
    AddonDef.Signature = 0x5DEED;
    AddonDef.APIVersion = NEXUS_API_VERSION;
    AddonDef.Name = "Nexus-Speedometer";
    AddonDef.Version.Major = V_MAJOR;
    AddonDef.Version.Minor = V_MINOR;
    AddonDef.Version.Build = V_BUILD;
    AddonDef.Version.Revision = V_REVISION;
    AddonDef.Author = "pushpayload";
    AddonDef.Description = "Nexus Speedometer";
    AddonDef.Load = AddonLoad;
    AddonDef.Unload = AddonUnload;
    AddonDef.Flags = EAddonFlags_None;

    /* not necessary if hosted on Raidcore, but shown anyway for the example
     * also useful as a backup resource */
    AddonDef.Provider = EUpdateProvider_GitHub;
    AddonDef.UpdateLink = "https://github.com/pushpayload/Nexus-Speedometer";

    return &AddonDef;
}

void AddonLoad(AddonAPI* aApi) {
    APIDefs = aApi;
    ImGui::SetCurrentContext((ImGuiContext*)APIDefs->ImguiContext);
    ImGui::SetAllocatorFunctions((void* (*)(size_t, void*))APIDefs->ImguiMalloc, (void (*)(void*, void*))APIDefs->ImguiFree); // on imgui 1.80+

    MumbleLink = (Mumble::Data*)APIDefs->DataLink.Get("DL_MUMBLE_LINK");
    NexusLink = (NexusLinkData*)APIDefs->DataLink.Get("DL_NEXUS_LINK");

    APIDefs->InputBinds.RegisterWithString(SPEED_TOGGLEVIS, ProcessKeybind, "(null)");

    APIDefs->Events.Subscribe(WINDOW_RESIZED, OnWindowResized);
    APIDefs->Events.Subscribe(MUMBLE_IDENTITY_UPDATED, OnMumbleIdentityUpdated);

    APIDefs->QuickAccess.AddContextMenu("QAS_SPEED", nullptr, AddonShortcut);

    APIDefs->Renderer.Register(ERenderType_Render, AddonRender);
    APIDefs->Renderer.Register(ERenderType_OptionsRender, AddonOptions);

    AddonPath = APIDefs->Paths.GetAddonDirectory("Nexus-Speedometer");
    SettingsPath = APIDefs->Paths.GetAddonDirectory("Nexus-Speedometer/settings.json");
    std::filesystem::create_directory(AddonPath);
    Settings::Load(SettingsPath);

    OnWindowResized(nullptr);
    AddonLog("Nexus-Speedometer loaded");
}

void AddonUnload() {
    AddonLog("Nexus-Speedometer unloading");

    // Cleanup speedreader
    g_speedReader.Unload();

    APIDefs->Renderer.Deregister(AddonOptions);
    APIDefs->Renderer.Deregister(AddonRender);

    APIDefs->QuickAccess.RemoveContextMenu("QAS_SPEED");

    APIDefs->Events.Unsubscribe(WINDOW_RESIZED, OnWindowResized);
    APIDefs->Events.Unsubscribe(MUMBLE_IDENTITY_UPDATED, OnMumbleIdentityUpdated);

    APIDefs->InputBinds.Deregister(SPEED_TOGGLEVIS);


    MumbleLink = nullptr;
    NexusLink = nullptr;

    Settings::Save(SettingsPath);
    AddonLog("Nexus-Speedometer unloaded");
}

void AddonRender() {
    if (!NexusLink || !MumbleLink || !MumbleIdentity || MumbleLink->Context.IsMapOpen || !NexusLink->IsGameplay) {
        return;
    }

    // Refresh speedreader addresses periodically
    static double lastRefreshTime = 0.0;
    double currentTime = ImGui::GetTime();
    if (currentTime - lastRefreshTime > 1.0f) { // Refresh every second
        if (g_speedReader.IsLoaded() && g_speedReader.RefreshAddresses) {
            g_speedReader.RefreshAddresses();
        }
        lastRefreshTime = currentTime;
    }

    if (Settings::IsIndicatorEnabled) {
        if (!Settings::IsIndicatorLocked) {
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 2.0f);
        }

        if (ImGui::Begin(
                "Nexus-Speedometer",
                (bool*)0,
                (Settings::IsIndicatorLocked ? (ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoInputs) : 0) | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoBringToFrontOnFocus |
                    ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoTitleBar | (Settings::IsIndicatorAutoResize ? ImGuiWindowFlags_AlwaysAutoResize : 0) |
                    (Settings::IsIndicatorTransparent && Settings::IsIndicatorLocked ? ImGuiWindowFlags_NoBackground : 0))) {
            // Add null checks for function pointers
            if (g_speedReader.IsLoaded() && g_speedReader.IsSpeedReaderValid && g_speedReader.GetCurrentSpeed && g_speedReader.GetMaxSpeed) {
                if (g_speedReader.IsSpeedReaderValid()) {
                    float currentSpeed = g_speedReader.GetCurrentSpeed();
                    float maxSpeed = g_speedReader.GetMaxSpeed();

                    if (Settings::ShowCurrentSpeed) {
                        ImGui::Text("Current Speed: %.1f", currentSpeed);
                    }
                    if (Settings::ShowMaxSpeed) {
                        ImGui::Text("Max Speed: %.1f", maxSpeed);
                    }
                    if (Settings::ShowCustomSpeed) {
                        std::string formattedText = FormatSpeed(Settings::SpeedFormat, currentSpeed, maxSpeed);
                        ImGui::Text(formattedText.c_str());
                    }
                } else {
                    ImGui::Text("Speed Reader Not Valid");
                }
            } else {
                ImGui::Text("Speed Reader Not Loaded");
            }
        }
        ImGui::End();

        if (!Settings::IsIndicatorLocked) {
            ImGui::PopStyleVar();
            ImGui::PopStyleColor();
        }
    }
}

void AddonOptions() {
    ImGui::Text("Nexus-Speedometer");

    if (ImGui::CollapsingHeader("Display Settings")) {
        ImGui::TextDisabled("Indicator");
        if (ImGui::Checkbox("Enabled##Indicator", &Settings::IsIndicatorEnabled)) {
            Settings::Settings[IS_INDICATOR_ENABLED] = Settings::IsIndicatorEnabled;
            Settings::Save(SettingsPath);
        }
        if (ImGui::Checkbox("Locked##Indicator", &Settings::IsIndicatorLocked)) {
            Settings::Settings[IS_INDICATOR_LOCKED] = Settings::IsIndicatorLocked;
            Settings::Save(SettingsPath);
        }
        if (ImGui::Checkbox("Auto Resize##Indicator", &Settings::IsIndicatorAutoResize)) {
            Settings::Settings[IS_INDICATOR_AUTO_RESIZE] = Settings::IsIndicatorAutoResize;
            Settings::Save(SettingsPath);
        }
        if (ImGui::Checkbox("Transparent##Indicator", &Settings::IsIndicatorTransparent)) {
            Settings::Settings[IS_INDICATOR_TRANSPARENT] = Settings::IsIndicatorTransparent;
            Settings::Save(SettingsPath);
        }

        ImGui::Separator();
        ImGui::TextDisabled("Speed Display");

        if (ImGui::Checkbox("Show Current Speed", &Settings::ShowCurrentSpeed)) {
            Settings::Settings[SHOW_CURRENT_SPEED] = Settings::ShowCurrentSpeed;
            Settings::Save(SettingsPath);
        }

        if (ImGui::Checkbox("Show Max Speed", &Settings::ShowMaxSpeed)) {
            Settings::Settings[SHOW_MAX_SPEED] = Settings::ShowMaxSpeed;
            Settings::Save(SettingsPath);
        }

        if (ImGui::Checkbox("Show Custom Speed", &Settings::ShowCustomSpeed)) {
            Settings::Settings[SHOW_CUSTOM_SPEED] = Settings::ShowCustomSpeed;
            Settings::Save(SettingsPath);
        }

        if (Settings::ShowCustomSpeed) {
            ImGui::Separator();
            ImGui::TextDisabled("Custom Format");

            static char formatBuffer[256];
            strncpy_s(formatBuffer, sizeof(formatBuffer), Settings::SpeedFormat.c_str(), sizeof(formatBuffer) - 1);
            if (ImGui::InputText("Format String", formatBuffer, sizeof(formatBuffer))) {
                Settings::SpeedFormat = formatBuffer;
                Settings::Settings[SPEED_FORMAT] = Settings::SpeedFormat;
                Settings::Save(SettingsPath);
            }

            ImGui::TextDisabled("Format placeholders:");
            ImGui::BulletText("{current} or {current:.1f} - Current speed");
            ImGui::BulletText("{max} or {max:.1f} - Maximum speed");
            ImGui::BulletText("Use .Nf to set decimal places (e.g. .2f)");
            ImGui::BulletText("Use \\n for new line");
        }

        if (ImGui::CollapsingHeader("Debug")) {
            int currentLogLevel = static_cast<int>(Settings::LogLevel);

            ImGui::PushItemWidth(100);
            if (ImGui::Combo("Log Level", &currentLogLevel, LOG_LEVEL_NAMES, LOG_LEVEL_COUNT)) {
                Settings::LogLevel = static_cast<ELogLevel>(currentLogLevel);
                Settings::Settings[LOG_LEVEL] = currentLogLevel;
                Settings::Save(SettingsPath);
                if (g_speedReader.IsLoaded() && g_speedReader.SetLogLevel) {
                    g_speedReader.SetLogLevel(Settings::LogLevel);
                }
                AddonLog(std::string("Log Level Changed to ") + LOG_LEVEL_NAMES[currentLogLevel], ELogLevel::ELogLevel_INFO);
            }
            ImGui::PopItemWidth();
            ImGui::Text("Speed Reader Status: %s", g_speedReader.IsLoaded() ? "Loaded" : "Not Loaded");
            ImGui::Text("Speed Reader Valid: %s", g_speedReader.IsSpeedReaderValid() ? "Yes" : "No");
            ImGui::Text("Current Speed: %.1f", g_speedReader.GetCurrentSpeed());
            ImGui::Text("Max Speed: %.1f", g_speedReader.GetMaxSpeed());
        }
    }
}

void AddonShortcut() {
    if (ImGui::Checkbox("Enabled##Indicator", &Settings::IsIndicatorEnabled)) {
        Settings::Settings[IS_INDICATOR_ENABLED] = Settings::IsIndicatorEnabled;
        Settings::Save(SettingsPath);
    }
    if (ImGui::Checkbox("Locked##Indicator", &Settings::IsIndicatorLocked)) {
        Settings::Settings[IS_INDICATOR_LOCKED] = Settings::IsIndicatorLocked;
        Settings::Save(SettingsPath);
    }
    if (ImGui::Checkbox("Auto Resize##Indicator", &Settings::IsIndicatorAutoResize)) {
        Settings::Settings[IS_INDICATOR_AUTO_RESIZE] = Settings::IsIndicatorAutoResize;
        Settings::Save(SettingsPath);
    }
    if (ImGui::Checkbox("Transparent##Indicator", &Settings::IsIndicatorTransparent)) {
        Settings::Settings[IS_INDICATOR_TRANSPARENT] = Settings::IsIndicatorTransparent;
        Settings::Save(SettingsPath);
    }

    if (ImGui::Checkbox("Show Current Speed", &Settings::ShowCurrentSpeed)) {
        Settings::Settings[SHOW_CURRENT_SPEED] = Settings::ShowCurrentSpeed;
        Settings::Save(SettingsPath);
    }

    if (ImGui::Checkbox("Show Max Speed", &Settings::ShowMaxSpeed)) {
        Settings::Settings[SHOW_MAX_SPEED] = Settings::ShowMaxSpeed;
        Settings::Save(SettingsPath);
    }

    if (ImGui::Checkbox("Show Custom Speed", &Settings::ShowCustomSpeed)) {
        Settings::Settings[SHOW_CUSTOM_SPEED] = Settings::ShowCustomSpeed;
        Settings::Save(SettingsPath);
    }
}

void ProcessKeybind(const char* aIdentifier, bool aIsRelease) {
    std::string str = aIdentifier;

    if (str == SPEED_TOGGLEVIS && aIsRelease) {
        Settings::IsIndicatorEnabled = !Settings::IsIndicatorEnabled;
    }
}

void OnWindowResized(void* aEventArgs) {}

void OnMumbleIdentityUpdated(void* aEventArgs) {
    MumbleIdentity = (Mumble::Identity*)aEventArgs;

    if (NexusLink->IsGameplay) {
        if (!g_speedReader.IsLoaded()) {
            if (!g_speedReader.Load(AddonLog, Settings::LogLevel ? Settings::LogLevel : ELogLevel::ELogLevel_INFO)) {
                AddonLog("Failed to load speedreader.dll", ELogLevel::ELogLevel_CRITICAL);
            }
        }
        if (g_speedReader.IsLoaded() && !g_speedReader.IsSpeedReaderValid()) {
            g_speedReader.RefreshAddresses();
        }
    }
}

void ReceiveTexture(const char* aIdentifier, Texture* aTexture) {
    std::string str = aIdentifier;

    // TODO: Implement texture handling
}

void AddonLog(const std::string& message, ELogLevel level) {
    APIDefs->Log(level, "Nexus-Speedometer", message.c_str());
}
