# Cragi Engine
project(Engine)

# Source files
file(GLOB_RECURSE ENGINE_SRC src/*.cpp include/*.h)

# Create a static library
add_library(Engine STATIC ${ENGINE_SRC})

# Include directories
target_include_directories(Engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

