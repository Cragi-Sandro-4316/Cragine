# --- CPM.cmake Inclusion ---
# Include CPM.cmake from your project's 'cmake' directory.
# This assumes you've manually downloaded get_cpm.cmake or CPM.cmake
# into the 'cmake/' subfolder of your project root.
include(cmake/CPM.cmake)
# --- End CPM.cmake Inclusion ---

# Cragi Engine
project(Engine)

CPMAddPackage(
    NAME spdlog
    GIT_REPOSITORY "https://github.com/gabime/spdlog.git"
    GIT_TAG        "v1.15.3" # <--- IMPORTANT: Specify a known, stable tag for spdlog
    OPTIONS        "SPDLOG_BUILD_EXAMPLES OFF" # Optional: turn off examples to speed up build
    OPTIONS        "SPDLOG_BUILD_TESTS OFF"    # Optional: turn off tests
)

CPMAddPackage(
    NAME glfw
    GIT_REPOSITORY  "https://github.com/glfw/glfw.git"
    GIT_TAG         "3.4"
)


include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Source files
file(GLOB_RECURSE ENGINE_SRC 
    src/*.cpp 
    src/*/*.cpp 
    src/*.h 
    src/*/*.h 
    src/*/*/*.h
    include/*.h
)

# Create a static library
add_library(Engine STATIC ${ENGINE_SRC})

# Link fmt to your Engine library
target_link_libraries(Engine PRIVATE spdlog)

# Link fmt to your Engine library
target_link_libraries(Engine PRIVATE glfw)

target_precompile_headers(Engine PRIVATE src/Cragine/CraginePCH.h)

# Include directories
target_include_directories(Engine PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    $<TARGET_PROPERTY:spdlog,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:glfw,INTERFACE_INCLUDE_DIRECTORIES>
)