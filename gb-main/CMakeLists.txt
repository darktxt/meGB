file(GLOB_RECURSE GB_MAIN "*.cpp")

find_path(SDL2_INCLUDE_DIR SDL2/SDL.h)
find_library(SDL2_LIBRARY SDL2)
find_library(SDL2_MAIN SDL2main)

add_executable(megb ${GB_MAIN})

target_include_directories(megb PUBLIC ${SDL2_INCLUDE_DIR})
target_include_directories(megb PUBLIC ../gb-lib)

target_link_libraries(megb PRIVATE megb-lib ${SDL2_LIBRARY} ${SDL2_MAIN} ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})