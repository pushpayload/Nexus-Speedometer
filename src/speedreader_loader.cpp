#include "speedreader_loader.h"
#include <filesystem>
#include "embedded_dll_loader.h"

SpeedReaderLoader::SpeedReaderLoader()
    : m_hModule(nullptr)
    , InitSpeedReader(nullptr)
    , CleanupSpeedReader(nullptr)
    , GetCurrentSpeed(nullptr)
    , GetMaxSpeed(nullptr)
    , IsSpeedReaderValid(nullptr)
    , RefreshAddresses(nullptr)
    , m_tempDllPath("") {}

SpeedReaderLoader::~SpeedReaderLoader() {
    Unload();
}

bool SpeedReaderLoader::Load(const std::string& dllPath) {
    if (m_hModule) return true; // Already loaded

    // Create temp path for the DLL
    char tempPath[MAX_PATH];
    GetTempPathA(MAX_PATH, tempPath);
    m_tempDllPath = std::string(tempPath) + "speedreader_temp.dll";

    // Extract and load the embedded DLL
    if (!EmbeddedDllLoader::ExtractAndLoadDll(GetModuleHandle(NULL), "SPEEDREADER_DLL", m_tempDllPath)) {
        return false;
    }

    m_hModule = LoadLibraryA(m_tempDllPath.c_str());
    if (!m_hModule) return false;

    // Load all function pointers
    InitSpeedReader = LoadFunction<bool (*)()>("InitSpeedReader");
    CleanupSpeedReader = LoadFunction<void (*)()>("CleanupSpeedReader");
    GetCurrentSpeed = LoadFunction<float (*)()>("GetCurrentSpeed");
    GetMaxSpeed = LoadFunction<float (*)()>("GetMaxSpeed");
    IsSpeedReaderValid = LoadFunction<bool (*)()>("IsSpeedReaderValid");
    RefreshAddresses = LoadFunction<bool (*)()>("RefreshAddresses");

    // Verify all functions were loaded
    if (!InitSpeedReader || !CleanupSpeedReader || !GetCurrentSpeed || !GetMaxSpeed || !IsSpeedReaderValid || !RefreshAddresses) {
        Unload();
        return false;
    }

    return true;
}

void SpeedReaderLoader::Unload() {
    if (m_hModule) {
        if (CleanupSpeedReader) {
            CleanupSpeedReader();
        }
        FreeLibrary(m_hModule);
        m_hModule = nullptr;
    }

    // Clear function pointers
    InitSpeedReader = nullptr;
    CleanupSpeedReader = nullptr;
    GetCurrentSpeed = nullptr;
    GetMaxSpeed = nullptr;
    IsSpeedReaderValid = nullptr;
    RefreshAddresses = nullptr;

    // Clean up the temporary DLL file
    if (!m_tempDllPath.empty()) {
        EmbeddedDllLoader::Cleanup(m_tempDllPath);
        m_tempDllPath.clear();
    }
}