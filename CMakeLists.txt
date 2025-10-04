cmake_minimum_required(VERSION 3.18)
project(Game)

# Set c++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Export compile commands
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Set opt level
set(CMAKE_CXX_FLAGS_RELEASE "-Wall -Os")

include_directories(Cragine/include)

# Add Engine project
add_subdirectory(Cragine)

# Add Game project
add_subdirectory(Project)
