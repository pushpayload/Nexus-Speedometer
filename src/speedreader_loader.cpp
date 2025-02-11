#include "speedreader_loader.h"
#include <filesystem>

SpeedReaderLoader::SpeedReaderLoader()
    : m_hModule(nullptr)
    , InitSpeedReader(nullptr)
    , CleanupSpeedReader(nullptr)
    , GetCurrentSpeed(nullptr)
    , GetMaxSpeed(nullptr)
    , IsSpeedReaderValid(nullptr)
    , RefreshAddresses(nullptr) {}

SpeedReaderLoader::~SpeedReaderLoader() {
    Unload();
}

bool SpeedReaderLoader::Load(const std::string& dllPath) {
    if (m_hModule) return true; // Already loaded

    // Get the directory of the current DLL
    char modulePath[MAX_PATH];
    if (!GetModuleFileNameA(GetModuleHandleA(NULL), modulePath, MAX_PATH)) {
        OutputDebugStringA("Failed to get module path");
        return false;
    }

    // Get the directory path
    std::filesystem::path moduleDir = std::filesystem::path(modulePath).parent_path();
    std::string speedreaderPath = (moduleDir / "speedreader.dll").string();

    // Try to load the DLL
    m_hModule = LoadLibraryA(speedreaderPath.c_str());
    if (!m_hModule) {
        DWORD error = GetLastError();
        char* errorMsg = nullptr;
        FormatMessageA(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            nullptr,
            error,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPSTR)&errorMsg,
            0,
            nullptr);

        std::string errorStr = "Failed to load DLL. Error code: " + std::to_string(error);
        if (errorMsg) {
            errorStr += " Message: " + std::string(errorMsg);
            LocalFree(errorMsg);
        }

        OutputDebugStringA(errorStr.c_str());
        OutputDebugStringA(("Attempted to load from: " + speedreaderPath).c_str());
        return false;
    }

    // Load all function pointers
    InitSpeedReader = LoadFunction<bool (*)()>("InitSpeedReader");
    CleanupSpeedReader = LoadFunction<void (*)()>("CleanupSpeedReader");
    GetCurrentSpeed = LoadFunction<float (*)()>("GetCurrentSpeed");
    GetMaxSpeed = LoadFunction<float (*)()>("GetMaxSpeed");
    IsSpeedReaderValid = LoadFunction<bool (*)()>("IsSpeedReaderValid");
    RefreshAddresses = LoadFunction<bool (*)()>("RefreshAddresses");

    // Verify all functions were loaded
    if (!InitSpeedReader || !CleanupSpeedReader || !GetCurrentSpeed || !GetMaxSpeed || !IsSpeedReaderValid || !RefreshAddresses) {
        OutputDebugStringA("Failed to load one or more functions");
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
}