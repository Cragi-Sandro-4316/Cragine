cmake_minimum_required(VERSION 3.18)
project(Cragine)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set the opt level
set(CMAKE_CXX_FLAGS_RELEASE "-Wall -Os")

include_directories(Engine/src/include)

# Add Engine project
add_subdirectory(Engine)

# Add Sandbox project
add_subdirectory(Game)

