# Sandbox Application
project(Game)

# Source files
file(GLOB_RECURSE GAME_SRC src/*.cpp)

# Create executable
add_executable(Game ${GAME_SRC})

# Link the Engine
target_link_libraries(Game PRIVATE Engine)
