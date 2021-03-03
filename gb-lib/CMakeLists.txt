file(GLOB_RECURSE GB_LIB "*.cpp")

find_path(SDL2_INCLUDE_DIR SDL2/SDL.h)
find_library(SDL2_LIBRARY SDL2)
find_library(SDL2_MAIN SDL2main)

add_library(megb-lib ${GB_LIB})
target_include_directories(megb-lib PUBLIC ${SDL2_INCLUDE_DIR})
target_link_libraries(megb-lib PRIVATE ${SDL2_LIBRARY} ${SDL2_MAIN} ${CMAKE_DL_LIBS})