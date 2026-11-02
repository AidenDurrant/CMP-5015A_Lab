cmake_minimum_required(VERSION 3.16)
project(rocketsim CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Every .cpp in src/ is part of the program. When you add a new file,
# re-run "cmake -B build" so the list is refreshed.
file(GLOB SOURCES CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/src/*.cpp)
add_executable(rocketsim ${SOURCES})

# Needed for std::thread in week 12 (no effect before then).
find_package(Threads REQUIRED)
target_link_libraries(rocketsim PRIVATE Threads::Threads)

if(MSVC)
    target_compile_options(rocketsim PRIVATE /W4)
else()
    target_compile_options(rocketsim PRIVATE -Wall -Wextra -Wpedantic)
endif()

# ---------------------------------------------------------------------------
# Live window (SFML 3). Downloaded and built the first time you configure, so
# the first "cmake -B build" needs internet access and takes a few minutes.
# If that is not possible on your machine:  cmake -B build -DROCKETSIM_LIVE=OFF
# The program then writes flight.svg only; every lab works the same either way.
# On Linux you need the usual X11/OpenGL/FreeType development packages
# (Ubuntu: libx11-dev libxrandr-dev libxcursor-dev libxi-dev libudev-dev
#  libgl1-mesa-dev libfreetype-dev). Windows and macOS need nothing extra.
# ---------------------------------------------------------------------------
option(ROCKETSIM_LIVE "Show the flight in a live window (fetches SFML)" ON)

if(ROCKETSIM_LIVE)
    include(FetchContent)
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
    set(SFML_BUILD_AUDIO OFF CACHE BOOL "" FORCE)
    set(SFML_BUILD_NETWORK OFF CACHE BOOL "" FORCE)
    set(SFML_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(SFML_BUILD_DOC OFF CACHE BOOL "" FORCE)
    FetchContent_Declare(SFML
        GIT_REPOSITORY https://github.com/SFML/SFML.git
        GIT_TAG 3.0.0
        GIT_SHALLOW ON)
    FetchContent_MakeAvailable(SFML)
    target_link_libraries(rocketsim PRIVATE SFML::Graphics)
    target_compile_definitions(rocketsim PRIVATE
        ROCKETSIM_LIVE
        ROCKETSIM_ASSET_DIR="${CMAKE_SOURCE_DIR}/assets")
endif()
