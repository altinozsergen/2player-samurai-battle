cmake_minimum_required(VERSION 3.16)
project(2-player-samurai-battle)

set(CMAKE_CXX_STANDARD 17)


include_directories("C:/raylib/raylib/src")


link_directories("C:/raylib/raylib/src")


add_executable(2-player-samurai-battle main.cpp)

target_link_libraries(2-player-samurai-battle raylib winmm gdi32 opengl32)
