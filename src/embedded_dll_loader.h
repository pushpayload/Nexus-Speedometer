#pragma once
#include <windows.h>
#include <string>

class EmbeddedDllLoader
{
public:
    static bool ExtractAndLoadDll(HMODULE hModule, const char* resourceName, const std::string& tempPath);
    static void Cleanup(const std::string& tempPath);

private:
    static bool ExtractResource(HMODULE hModule, const char* resourceName, const std::string& outputPath);
};