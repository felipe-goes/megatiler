cmake_minimum_required(VERSION 3.22.2)
# command: cmake . -DCMAKE_EXPORT_COMPILE_COMMANDS=1 && rm -rf cmake_install.cmake CMakeCache.txt CMakeFiles/ Makefile
# set the project name
project(template VERSION 1.0)

# Project
link_directories(./src)
include_directories(./src)

link_directories(./inc)
include_directories(./inc)

link_directories(./res)
include_directories(./res)

# SGDK
link_directories(/opt/SGDK/inc)
include_directories(/opt/SGDK/inc)

link_directories(/opt/SGDK/inc/ext)
include_directories(/opt/SGDK/inc/ext)

link_directories(/opt/SGDK/inc/ext/mw)
include_directories(/opt/SGDK/inc/ext/mw)

link_directories(/opt/SGDK/src)
include_directories(/opt/SGDK/src)

link_directories(/opt/SGDK/src/boot)
include_directories(/opt/SGDK/src/boot)

link_directories(/opt/SGDK/src/ext)
include_directories(/opt/SGDK/src/ext)

link_directories(/opt/SGDK/src/ext/mw)
include_directories(/opt/SGDK/src/ext/mw)

link_directories(/opt/SGDK/res)
include_directories(/opt/SGDK/res)

file(GLOB_RECURSE SOURCES "src/*.c" main.c)
add_executable(template ${SOURCES})
target_link_libraries(template libs)
