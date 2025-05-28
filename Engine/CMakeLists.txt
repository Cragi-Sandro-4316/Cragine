# Cragi Engine
project(Engine)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Source files
file(GLOB_RECURSE ENGINE_SRC src/*.cpp src/*/*.cpp src/*.h src/*/*.h include/*.h)

# Create a static library
add_library(Engine STATIC ${ENGINE_SRC})


# Include directories
target_include_directories(Engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

