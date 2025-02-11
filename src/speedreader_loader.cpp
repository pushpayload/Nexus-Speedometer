#include "speedreader_loader.h"

// Initialize static member
void (*SpeedReaderLoader::s_logCallback)(const std::string&, ELogLevel) = nullptr;

void SpeedReaderLoader::LogWrapperFunc(const char* message, ESpeedReaderLogLevel level) {
    if (!s_logCallback) return;

    ELogLevel mappedLevel;
    switch (level) {
    case ESpeedReaderLogLevel_INFO: mappedLevel = ELogLevel_INFO; break;
    case ESpeedReaderLogLevel_WARNING: mappedLevel = ELogLevel_WARNING; break;
    case ESpeedReaderLogLevel_CRITICAL: mappedLevel = ELogLevel_CRITICAL; break;
    case ESpeedReaderLogLevel_DEBUG: mappedLevel = ELogLevel_DEBUG; break;
    default: mappedLevel = ELogLevel_INFO;
    }
    s_logCallback(std::string(message), mappedLevel);
}

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

bool SpeedReaderLoader::Load(void (*logCallback)(const std::string&, ELogLevel)) {
    if (m_hModule) return true; // Already loaded

    // Store the log callback
    s_logCallback = logCallback;

    m_hModule = LoadLibraryA("speedreader.dll");
    if (!m_hModule) return false;

    // Load all function pointers
    InitSpeedReader = LoadFunction<decltype(InitSpeedReader)>("InitSpeedReader");
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

    // Initialize with logging callback
    return InitSpeedReader(LogWrapperFunc);
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