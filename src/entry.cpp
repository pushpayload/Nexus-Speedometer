#include "settings.h"
#include "shared.h"

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

const char* HW_TOGGLEVIS = "KB_HW_TOGGLEVIS";
const char* WINDOW_RESIZED = "EV_WINDOW_RESIZED";
const char* MUMBLE_IDENTITY_UPDATED = "EV_MUMBLE_IDENTITY_UPDATED";


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
    AddonDef.Signature = 0x311001D;
    AddonDef.APIVersion = NEXUS_API_VERSION;
    AddonDef.Name = "Nexus-HelloWorld";
    AddonDef.Version.Major = V_MAJOR;
    AddonDef.Version.Minor = V_MINOR;
    AddonDef.Version.Build = V_BUILD;
    AddonDef.Version.Revision = V_REVISION;
    AddonDef.Author = "pushpayload";
    AddonDef.Description = "Boilerplate Nexus addon";
    AddonDef.Load = AddonLoad;
    AddonDef.Unload = AddonUnload;
    AddonDef.Flags = EAddonFlags_None;

    /* not necessary if hosted on Raidcore, but shown anyway for the example
     * also useful as a backup resource */
    AddonDef.Provider = EUpdateProvider_GitHub;
    AddonDef.UpdateLink = "https://github.com/pushpayload/Nexus-HelloWorld";

    return &AddonDef;
}

void AddonLoad(AddonAPI* aApi) {
    APIDefs = aApi;
    ImGui::SetCurrentContext((ImGuiContext*)APIDefs->ImguiContext);
    ImGui::SetAllocatorFunctions((void* (*)(size_t, void*))APIDefs->ImguiMalloc, (void (*)(void*, void*))APIDefs->ImguiFree); // on imgui 1.80+

    MumbleLink = (Mumble::Data*)APIDefs->DataLink.Get("DL_MUMBLE_LINK");
    NexusLink = (NexusLinkData*)APIDefs->DataLink.Get("DL_NEXUS_LINK");

    APIDefs->InputBinds.RegisterWithString(HW_TOGGLEVIS, ProcessKeybind, "(null)");

    APIDefs->Events.Subscribe(WINDOW_RESIZED, OnWindowResized);
    APIDefs->Events.Subscribe(MUMBLE_IDENTITY_UPDATED, OnMumbleIdentityUpdated);

    APIDefs->QuickAccess.AddContextMenu("QAS_HW", nullptr, AddonShortcut);

    APIDefs->Renderer.Register(ERenderType_Render, AddonRender);
    APIDefs->Renderer.Register(ERenderType_OptionsRender, AddonOptions);

    AddonPath = APIDefs->Paths.GetAddonDirectory("Nexus-HelloWorld");
    SettingsPath = APIDefs->Paths.GetAddonDirectory("Nexus-HelloWorld/settings.json");
    std::filesystem::create_directory(AddonPath);
    Settings::Load(SettingsPath);

    OnWindowResized(nullptr); // initialise self
    AddonLog("Nexus-HelloWorld loaded");
}

void AddonUnload() {
    AddonLog("Nexus-HelloWorld unloading");

    APIDefs->Renderer.Deregister(AddonOptions);
    APIDefs->Renderer.Deregister(AddonRender);

    APIDefs->QuickAccess.RemoveContextMenu("QAS_HW");

    APIDefs->Events.Unsubscribe(WINDOW_RESIZED, OnWindowResized);
    APIDefs->Events.Unsubscribe(MUMBLE_IDENTITY_UPDATED, OnMumbleIdentityUpdated);

    APIDefs->InputBinds.Deregister(HW_TOGGLEVIS);


    MumbleLink = nullptr;
    NexusLink = nullptr;

    Settings::Save(SettingsPath);
    AddonLog("Nexus-HelloWorld unloaded");
}

void AddonRender() {
    if (!NexusLink || !MumbleLink || !MumbleIdentity || MumbleLink->Context.IsMapOpen || !NexusLink->IsGameplay) {
        return;
    }
    if (Settings::IsIndicatorEnabled) {
        if (!Settings::IsIndicatorLocked) {
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 2.0f);
        }
        if (ImGui::Begin(
                "Nexus-HelloWorld",
                (bool*)0,
                (Settings::IsIndicatorLocked ? (ImGuiWindowFlags_NoMove) : 0) | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoScrollbar |
                    ImGuiWindowFlags_NoTitleBar | (Settings::IsIndicatorAutoResize ? ImGuiWindowFlags_AlwaysAutoResize : 0))) {
            ImGui::Text("Hello World!");
        }
        ImGui::End();
        if (!Settings::IsIndicatorLocked) {
            ImGui::PopStyleVar();
            ImGui::PopStyleColor();
        }
    }
}

void AddonOptions() {
    ImGui::Text("Nexus-HelloWorld");
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
}

void ProcessKeybind(const char* aIdentifier, bool aIsRelease) {
    std::string str = aIdentifier;

    if (str == HW_TOGGLEVIS && aIsRelease) {
        Settings::IsIndicatorEnabled = !Settings::IsIndicatorEnabled;
    }
}

void OnWindowResized(void* aEventArgs) {}

void OnMumbleIdentityUpdated(void* aEventArgs) {
    MumbleIdentity = (Mumble::Identity*)aEventArgs;
}

void ReceiveTexture(const char* aIdentifier, Texture* aTexture) {
    std::string str = aIdentifier;

    // TODO: Implement texture handling
}

void AddonLog(const std::string& message, ELogLevel level) {
    APIDefs->Log(level, "Nexus-HelloWorld", message.c_str());
}
