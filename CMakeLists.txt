# CMakeList.txt : CMake project for Nexus-HelloWorld, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

# Set up cross compilation for Windows - MUST be before project()
if (CMAKE_HOST_SYSTEM_NAME STREQUAL "Linux")
  set(CMAKE_SYSTEM_NAME Windows)
  set(CMAKE_SYSTEM_PROCESSOR x86_64)
  set(TOOLCHAIN_PREFIX x86_64-w64-mingw32)
  
  set(CMAKE_C_COMPILER ${TOOLCHAIN_PREFIX}-gcc-posix)
  set(CMAKE_CXX_COMPILER ${TOOLCHAIN_PREFIX}-g++-posix)
  set(CMAKE_RC_COMPILER ${TOOLCHAIN_PREFIX}-windres)
  
  # Add static linking flags
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++ -static")
  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -static-libgcc -static-libstdc++ -static")
  
  # Set the root path for finding Windows libraries/headers
  set(CMAKE_FIND_ROOT_PATH /usr/${TOOLCHAIN_PREFIX})
  set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
  set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
  set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
endif()

project ("Nexus-HelloWorld")

# Specify C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Output directory settings
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Set DLL output name
set(OUTPUT_NAME "Nexus-HelloWorld")

# Add ImGui source files
set(IMGUI_SOURCES
    src/imgui/imgui.cpp
    src/imgui/imgui_demo.cpp
    src/imgui/imgui_draw.cpp
    src/imgui/imgui_tables.cpp
    src/imgui/imgui_widgets.cpp
)

# Add source files
set(SOURCES
    src/entry.cpp
    src/shared.cpp
    src/settings.cpp
    ${IMGUI_SOURCES}  # Add ImGui sources to the build
)

# Add header files
set(HEADERS
  src/shared.h
  src/settings.h
  src/imgui/imgui.h
  src/mumble/Mumble.h
  src/nexus/Nexus.h
  src/nlohmann/json.hpp
  src/version.h
)

# Create version.h before build (windows powershell, linux bash)
if (CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
  add_custom_command(
    OUTPUT ${CMAKE_SOURCE_DIR}/src/version.h
    COMMAND powershell.exe -ExecutionPolicy Bypass -File ${CMAKE_SOURCE_DIR}/scripts/generate_version.ps1
    DEPENDS ${CMAKE_SOURCE_DIR}/scripts/generate_version.ps1
  )
elseif (CMAKE_HOST_SYSTEM_NAME STREQUAL "Linux")
  add_custom_command(
    OUTPUT ${CMAKE_SOURCE_DIR}/src/version.h
    COMMAND bash ${CMAKE_SOURCE_DIR}/scripts/generate_version.sh ${CMAKE_SOURCE_DIR}/src/version.h
    DEPENDS ${CMAKE_SOURCE_DIR}/scripts/generate_version.sh
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  )
endif()

# Create a custom target for version.h
add_custom_target(generate_version DEPENDS ${CMAKE_SOURCE_DIR}/src/version.h)

# Add library target
add_library(${PROJECT_NAME} SHARED 
    ${SOURCES} 
    ${HEADERS}
    ${RESOURCES}
    ${CMAKE_SOURCE_DIR}/src/version.h
)

# Make sure version.h is generated before building the library
add_dependencies(${PROJECT_NAME} generate_version)

# Set output name
set_target_properties(${PROJECT_NAME} PROPERTIES 
    PREFIX ""
    SUFFIX ".dll"
    OUTPUT_NAME ${OUTPUT_NAME}
)

# Add include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/imgui
    ${CMAKE_SOURCE_DIR}/src/mumble
    ${CMAKE_SOURCE_DIR}/src/nexus
    ${CMAKE_SOURCE_DIR}/src/nlohmann
    $<$<STREQUAL:${CMAKE_HOST_SYSTEM_NAME},Linux>:/usr/${TOOLCHAIN_PREFIX}/include>
)