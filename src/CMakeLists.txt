set(BINARY ${CMAKE_PROJECT_NAME})

file(GLOB_RECURSE SOURCES LIST_DIRECTORIES true *.h *.cpp)

set(SOURCES ${SOURCES})

add_executable(${BINARY} ${SOURCES})
target_compile_options(${BINARY} PUBLIC -Wall -Wextra)
target_link_libraries(${BINARY} ${SDL2_LIBRARIES} Boost::program_options)
