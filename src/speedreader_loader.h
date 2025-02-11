#pragma once
#include <windows.h>
#include <string>

class SpeedReaderLoader
{
public:
    SpeedReaderLoader();
    ~SpeedReaderLoader();

    bool Load(const std::string& dllPath = "speedreader.dll");
    void Unload();

    // Function pointers matching the DLL exports
    bool (*InitSpeedReader)();
    void (*CleanupSpeedReader)();
    float (*GetCurrentSpeed)();
    float (*GetMaxSpeed)();
    bool (*IsSpeedReaderValid)();
    bool (*RefreshAddresses)();

    bool IsLoaded() const { return m_hModule != nullptr; }

private:
    HMODULE m_hModule;
    std::string m_tempDllPath;

    // Helper to load function pointers
    template <typename T>
    T LoadFunction(const char* name) {
        return reinterpret_cast<T>(GetProcAddress(m_hModule, name));
    }
};