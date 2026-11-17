cmake_minimum_required(VERSION 3.11)
project(setup)

# Require modern C++ (SFML 2.6.x is happy with C++14)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#### Setup Directories ####
#Main output directory
SET(OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/")
# Ouput all DLLs from all libs into main build folder
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIRECTORY})

#### Add External Dependencies ####
# The game only needs graphics/window/system, so skip building audio and network.
set(SFML_BUILD_AUDIO   OFF CACHE BOOL "" FORCE)
set(SFML_BUILD_NETWORK OFF CACHE BOOL "" FORCE)
add_subdirectory("lib/SFML")
set(SFML_INCS "lib/SFML/include")
link_directories("${CMAKE_BINARY_DIR}/lib/SFML/lib")

#### Fishing Game ####
add_executable(fish_off
    main.cpp
    Game.cpp
    Pond.cpp
    Rod.cpp)
target_include_directories(fish_off PRIVATE ${SFML_INCS} ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(fish_off sfml-graphics)
