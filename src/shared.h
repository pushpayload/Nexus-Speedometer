#pragma once

#ifndef SHARED_H
    #define SHARED_H

    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>

    #include <filesystem>
    #include <fstream>
    #include <mutex>
    #include <string>
    #include <vector>

    #include "imgui/imgui.h"
    #include "mumble/Mumble.h"
    #include "nexus/Nexus.h"

extern AddonAPI* APIDefs;

extern Mumble::Data* MumbleLink;
extern Mumble::Identity* MumbleIdentity;
extern NexusLinkData* NexusLink;

#endif