# Sandbox Application
project(Project)

# Source files
file(GLOB_RECURSE PROJECT_SRC CMAKE_CONFIGURE_DEPENDS src/*.cpp)

# Create executable
add_executable(Project ${PROJECT_SRC})

find_package(Vulkan REQUIRED)

# Link the Engine
target_link_libraries(Project PRIVATE Cragine)
