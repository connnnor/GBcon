cmake_minimum_required(VERSION 3.7 FATAL_ERROR)
project(GBcon)

# boost stuff
set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost 1.50 REQUIRED COMPONENTS program_options)

find_package(SDL2 REQUIRED)

set(CMAKE_CXX_STANDARD 14)

include_directories(include ${SDL2_INCLUDE_DIRS} SYSTEM ${Boost_INCLUDE_DIR})

add_subdirectory(src)

install(TARGETS GBcon DESTINATION bin)
