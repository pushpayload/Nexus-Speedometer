#pragma once
#include "shared.h"

#ifdef SPEEDREADER_EXPORTS
    #define SPEEDREADER_API __declspec(dllexport)
#else
    #define SPEEDREADER_API __declspec(dllimport)
#endif

// Define the logging callback type
typedef void (*SpeedReaderLogCallback)(const char* message, ELogLevel level);

extern "C" {
// Initialize the speed reader - returns true if successful
SPEEDREADER_API bool InitSpeedReader(SpeedReaderLogCallback logCallback = nullptr, ELogLevel initLogLevel = ELogLevel_INFO);

// Cleanup resources
SPEEDREADER_API void CleanupSpeedReader();

// Get the current speed value
SPEEDREADER_API float GetCurrentSpeed();

// Get the max speed value
SPEEDREADER_API float GetMaxSpeed();

// Check if speed reader is initialized and valid
SPEEDREADER_API bool IsSpeedReaderValid();

// Refresh addresses (call this if values stop working)
SPEEDREADER_API bool RefreshAddresses();

// Set the log level
SPEEDREADER_API void SetLogLevel(ELogLevel level);
}