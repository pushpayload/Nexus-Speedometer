#pragma once
#include <windows.h>
#include <string>
#include "shared.h" // For ELogLevel
#include "speedreader.h" // For SpeedReaderLogCallback

class SpeedReaderLoader
{
public:
    SpeedReaderLoader();
    ~SpeedReaderLoader();

    bool Load(void (*logCallback)(const std::string&, ELogLevel) = nullptr, ELogLevel initLogLevel = ELogLevel::ELogLevel_INFO);
    void Unload();

    // Function pointers matching the DLL exports
    bool (*InitSpeedReader)(SpeedReaderLogCallback, ELogLevel);
    void (*CleanupSpeedReader)();
    float (*GetCurrentSpeed)();
    float (*GetMaxSpeed)();
    bool (*IsSpeedReaderValid)();
    bool (*RefreshAddresses)();
    void (*SetLogLevel)(ELogLevel);
    const char* (*GetDllVersion)();
    bool IsLoaded() const { return m_hModule != nullptr; }

private:
    HMODULE m_hModule;

    // Static callback for DLL logging
    static void (*s_logCallback)(const std::string&, ELogLevel);
    static void LogWrapperFunc(const char* message, ELogLevel level);

    // Helper to load function pointers
    template <typename T>
    T LoadFunction(const char* name) {
        return reinterpret_cast<T>(GetProcAddress(m_hModule, name));
    }
};