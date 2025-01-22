# Cragi Engine
project(Engine)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Add the spdlog submodule path as an external dependency
add_subdirectory(libs/spdlog)

# Add the glfw submodule path as an external dependency
add_subdirectory(libs/glfw)

# Add the glad submodule path as an external dependency
add_subdirectory(libs/glad)

# Add the ImGui submodule path as an external dependency
add_subdirectory(libs/imgui) 


# Source files
file(GLOB_RECURSE ENGINE_SRC src/*.cpp src/*/*.cpp include/*.h include/*/*.h)

# Create a static library
add_library(Engine STATIC ${ENGINE_SRC})

# Link spdlog to the Engine
target_link_libraries(Engine PRIVATE spdlog::spdlog glfw glad imgui)

# Set the path to the precompiled header
target_precompile_headers(Engine PRIVATE include/CraginePCH.h)

# Include directories
target_include_directories(Engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Include the header files for spdlog
target_include_directories(Engine PUBLIC libs/spdlog/include)

