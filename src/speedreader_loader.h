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

    bool Load(void (*logCallback)(const std::string&, ELogLevel) = nullptr);
    void Unload();

    // Function pointers matching the DLL exports
    bool (*InitSpeedReader)(SpeedReaderLogCallback);
    void (*CleanupSpeedReader)();
    float (*GetCurrentSpeed)();
    float (*GetMaxSpeed)();
    bool (*IsSpeedReaderValid)();
    bool (*RefreshAddresses)();

    bool IsLoaded() const { return m_hModule != nullptr; }

private:
    HMODULE m_hModule;

    // Static callback for DLL logging
    static void (*s_logCallback)(const std::string&, ELogLevel);
    static void LogWrapperFunc(const char* message, ESpeedReaderLogLevel level);

    // Helper to load function pointers
    template <typename T>
    T LoadFunction(const char* name) {
        return reinterpret_cast<T>(GetProcAddress(m_hModule, name));
    }
};