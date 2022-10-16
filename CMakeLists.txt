cmake_minimum_required(VERSION 3.25)
project(meGB)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
add_subdirectory(gb-lib)
add_subdirectory(gb-test)
add_subdirectory(gb-main)

file(COPY ${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/bin/sdl2.dll DESTINATION ${CMAKE_BINARY_DIR}/bin)