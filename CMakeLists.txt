cmake_minimum_required(VERSION 3.5)
project(solid2)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Setup vcpkg integration
set(VCPKG_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/external/vcpkg")
set(CMAKE_TOOLCHAIN_FILE "${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")

# Vcpkg commands
set(VCPKG_COMMAND "vcpkg")
set(VCPKG_BOOTSTRAP_COMMAND "bootstrap-vcpkg.sh")
if(WIN32)
    set(VCPKG_COMMAND "vcpkg.exe")
    set(VCPKG_BOOTSTRAP_COMMAND "bootstrap-vcpkg.bat")
endif()

# Bootstrap vcpkg if necessary
set(VCPKG_EXECUTABLE "${VCPKG_ROOT}/${VCPKG_COMMAND}")
if(NOT EXISTS "${VCPKG_EXECUTABLE}")
    message(STATUS "Bootstrapping vcpkg")
    execute_process(
        COMMAND "${VCPKG_ROOT}/${VCPKG_BOOTSTRAP_COMMAND}"
        WORKING_DIRECTORY "${VCPKG_ROOT}"
        RESULT_VARIABLE VCPKG_BOOTSTRAP_RESULT
    )
    if(NOT VCPKG_BOOTSTRAP_RESULT EQUAL 0)
        message(FATAL_ERROR "Failed to bootstrap vcpkg")
    endif()
endif()

# Determine platform specifics
if(WIN32)
    set(VCPKG_TRIPLET "x64-windows")
elseif(APPLE)
    execute_process(
        COMMAND uname -m
        OUTPUT_VARIABLE ARCH
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    if(ARCH STREQUAL "arm64")
        set(VCPKG_TRIPLET "arm64-osx")
    else()
        set(VCPKG_TRIPLET "x64-osx")
    endif()
else()
    set(VCPKG_TRIPLET "x64-linux")
endif()

# Find and install required packages
find_package(raylib CONFIG QUIET)
if(NOT raylib_FOUND)
    message(STATUS "Installing raylib")
    execute_process(
        COMMAND "${VCPKG_EXECUTABLE}" install raylib
        WORKING_DIRECTORY "${VCPKG_ROOT}"
        RESULT_VARIABLE VCPKG_INSTALL_RESULT
    )
    if(NOT VCPKG_INSTALL_RESULT EQUAL 0)
        message(FATAL_ERROR "Failed to install raylib")
    endif()
    set(raylib_DIR "${VCPKG_ROOT}/installed/${VCPKG_TRIPLET}/share/raylib")
    set(raylib_INCLUDE_DIR "${VCPKG_ROOT}/installed/${VCPKG_TRIPLET}/include")
    set(raylib_LIBRARY "${VCPKG_ROOT}/installed/${VCPKG_TRIPLET}/lib/libraylib.a")
    set(glfw3_DIR "${VCPKG_ROOT}/installed/${VCPKG_TRIPLET}/share/glfw3")
    set(glfw3_LIBRARY "${VCPKG_ROOT}/installed/${VCPKG_TRIPLET}/lib/libglfw3.a")
endif()

find_package(enet CONFIG QUIET)
if(NOT enet_FOUND)
    message(STATUS "Installing enet")
    execute_process(
        COMMAND "${VCPKG_EXECUTABLE}" install enet
        WORKING_DIRECTORY "${VCPKG_ROOT}"
        RESULT_VARIABLE VCPKG_INSTALL_RESULT
    )
    if(NOT VCPKG_INSTALL_RESULT EQUAL 0)
        message(FATAL_ERROR "Failed to install enet")
    endif()
    set(unofficial-enet_DIR "${VCPKG_ROOT}/installed/${VCPKG_TRIPLET}/share/unofficial-enet")
    set(unofficial-enet_INCLUDE_DIR "${VCPKG_ROOT}/installed/${VCPKG_TRIPLET}/include")
    set(unofficial-enet_LIB_DIR "${VCPKG_ROOT}/installed/${VCPKG_TRIPLET}/lib/libenet.a")
endif()

find_package(unofficial-enet CONFIG REQUIRED)
find_package(raylib CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(OpenGL REQUIRED)

# Platform-specific libraries
if(WIN32)
    set(LIBS raylib glfw opengl32 gdi32 winmm)
elseif(APPLE)
    set(LIBS raylib glfw OpenGL::GL "-framework IOKit" "-framework Cocoa" "-framework OpenGL")
else()
    option(USE_WAYLAND "Use Wayland instead of X11" OFF)
    if(USE_WAYLAND)
        set(LIBS raylib glfw OpenGL::GL wayland-client wayland-cursor wayland-egl)
    else()
        set(LIBS raylib glfw OpenGL::GL X11 Xrandr Xi Xinerama Xxf86vm Xcursor)
    endif()
endif()

# Set source files and include directories
file(GLOB_RECURSE SOURCES "src/*.cpp")
include_directories("src")

# Create executable and link libraries
add_executable(solid2 ${SOURCES})
target_link_libraries(solid2 PRIVATE raylib unofficial::enet::enet glfw ${LIBS})
