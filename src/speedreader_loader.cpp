#include "speedreader_loader.h"

// Initialize static member
void (*SpeedReaderLoader::s_logCallback)(const std::string&, ELogLevel) = nullptr;

void SpeedReaderLoader::LogWrapperFunc(const char* message, ELogLevel level) {
    if (!s_logCallback) return;
    s_logCallback(std::string(message), level);
}

SpeedReaderLoader::SpeedReaderLoader()
    : m_hModule(nullptr)
    , InitSpeedReader(nullptr)
    , CleanupSpeedReader(nullptr)
    , GetCurrentSpeed(nullptr)
    , GetMaxSpeed(nullptr)
    , IsSpeedReaderValid(nullptr)
    , RefreshAddresses(nullptr)
    , SetLogLevel(nullptr)
    , GetDllVersion(nullptr) {}

SpeedReaderLoader::~SpeedReaderLoader() {
    Unload();
}

bool SpeedReaderLoader::Load(void (*logCallback)(const std::string&, ELogLevel), ELogLevel initLogLevel) {
    if (m_hModule) return true; // Already loaded

    // Store the log callback
    s_logCallback = logCallback;

    // Try loading from addons folder first
    m_hModule = LoadLibraryA("addons/speedreader.dll");
    if (!m_hModule) {
        // Fall back to main folder
        m_hModule = LoadLibraryA("speedreader.dll");
        if (!m_hModule) return false;
    }

    // Load all function pointers
    InitSpeedReader = LoadFunction<decltype(InitSpeedReader)>("InitSpeedReader");
    CleanupSpeedReader = LoadFunction<void (*)()>("CleanupSpeedReader");
    GetCurrentSpeed = LoadFunction<float (*)()>("GetCurrentSpeed");
    GetMaxSpeed = LoadFunction<float (*)()>("GetMaxSpeed");
    IsSpeedReaderValid = LoadFunction<bool (*)()>("IsSpeedReaderValid");
    RefreshAddresses = LoadFunction<bool (*)()>("RefreshAddresses");
    SetLogLevel = LoadFunction<void (*)(ELogLevel)>("SetLogLevel");
    GetDllVersion = LoadFunction<const char* (*)()>("GetDllVersion");

    // Verify all functions were loaded
    if (!InitSpeedReader || !CleanupSpeedReader || !GetCurrentSpeed || !GetMaxSpeed || !IsSpeedReaderValid || !RefreshAddresses || !SetLogLevel) {
        Unload();
        return false;
    };

    // Initialize with logging callback
    return InitSpeedReader(LogWrapperFunc, initLogLevel);
}

void SpeedReaderLoader::Unload() {
    if (m_hModule) {
        if (CleanupSpeedReader) {
            CleanupSpeedReader();
        }
        FreeLibrary(m_hModule);
        m_hModule = nullptr;
    }

    // Clear the log callback
    s_logCallback = nullptr;

    // Clear function pointers
    InitSpeedReader = nullptr;
    CleanupSpeedReader = nullptr;
    GetCurrentSpeed = nullptr;
    GetMaxSpeed = nullptr;
    IsSpeedReaderValid = nullptr;
    RefreshAddresses = nullptr;
}