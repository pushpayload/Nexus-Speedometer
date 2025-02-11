#include "embedded_dll_loader.h"
#include <fstream>

bool EmbeddedDllLoader::ExtractAndLoadDll(HMODULE hModule, const char* resourceName, const std::string& tempPath) {
    if (!ExtractResource(hModule, resourceName, tempPath)) {
        return false;
    }
    return true;
}

void EmbeddedDllLoader::Cleanup(const std::string& tempPath) {
    DeleteFileA(tempPath.c_str());
}

bool EmbeddedDllLoader::ExtractResource(HMODULE hModule, const char* resourceName, const std::string& outputPath) {
    HRSRC hResource = FindResourceA(hModule, resourceName, RT_RCDATA);
    if (!hResource) return false;

    HGLOBAL hLoadedResource = LoadResource(hModule, hResource);
    if (!hLoadedResource) return false;

    LPVOID pLockedResource = LockResource(hLoadedResource);
    if (!pLockedResource) return false;

    DWORD dwResourceSize = SizeofResource(hModule, hResource);
    if (!dwResourceSize) return false;

    std::ofstream file(outputPath, std::ios::binary);
    if (!file) return false;

    file.write(static_cast<const char*>(pLockedResource), dwResourceSize);
    file.close();

    return true;
}