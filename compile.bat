@ECHO OFF
SETLOCAL ENABLEEXTENSIONS ENABLEDELAYEDEXPANSION
SET VCPKG_DIR=C:\Users\txt_len\Documents\me\2021\projects\gameboyemulator\vcpkg
SET CMAKE_EXE="C:\Program Files\CMake\bin\cmake.exe"
SET VSCMD_ARG_TGT_ARCH=x64

RMDIR /s/q build
MKDIR build
PUSHD build

%VCPKG_DIR%\vcpkg.exe install sdl2:%VSCMD_ARG_TGT_ARCH%-windows

%CMAKE_EXE% .. "-DCMAKE_TOOLCHAIN_FILE=%VCPKG_DIR%\scripts\buildsystems\vcpkg.cmake" -DVCPKG_TARGET_TRIPLET=%VSCMD_ARG_TGT_ARCH%-windows -G "Visual Studio 16 2019"

ENDLOCAL