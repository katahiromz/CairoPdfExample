cmake_minimum_required(VERSION 3.10)

project(CairoPdfExample)

set(CAIRO_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../cairo-1.18.2")

link_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(CAIRO_INCLUDE_DIRS ${CAIRO_DIR}/src ${CAIRO_DIR}/build/src)
set(CAIRO_LIBRARIES libcairo-2.dll libpng12.dll zlib1.dll) # Borrowed from gtk-2.12.9-win32-2.exe

add_executable(CairoPdfExample CairoPdfExample.cpp)
target_include_directories(CairoPdfExample PRIVATE ${CAIRO_INCLUDE_DIRS})
target_link_libraries(CairoPdfExample PRIVATE ${CAIRO_LIBRARIES})
