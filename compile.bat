@ECHO OFF
SETLOCAL ENABLEEXTENSIONS ENABLEDELAYEDEXPANSION
SET VCPKG_DIR=D:\Source\Repos\darktxt\meGB\vcpkg
SET CMAKE_EXE="D:\Program Files\CMake\bin\cmake.exe"
SET Visual_Studio="Visual Studio 17 2022"
SET VSCMD_ARG_TGT_ARCH=x64

RMDIR /s/q build
MKDIR build
PUSHD build

%VCPKG_DIR%\vcpkg.exe install sdl2:%VSCMD_ARG_TGT_ARCH%-windows

%CMAKE_EXE% .. "-DCMAKE_TOOLCHAIN_FILE=%VCPKG_DIR%\scripts\buildsystems\vcpkg.cmake" -DVCPKG_TARGET_TRIPLET=%VSCMD_ARG_TGT_ARCH%-windows -G %Visual_Studio%

ENDLOCAL
