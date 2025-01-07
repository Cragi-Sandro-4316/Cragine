# Sandbox Application
project(Sandbox)

# Source files
file(GLOB_RECURSE SANDBOX_SRC src/*.cpp)

# Create executable
add_executable(Sandbox ${SANDBOX_SRC})

# Link the Engine
target_link_libraries(Sandbox PRIVATE Engine)
