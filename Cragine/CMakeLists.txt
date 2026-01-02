project(Cragine)

# Source files
file(GLOB_RECURSE ENGINE_SRC CMAKE_CONFIGURE_DEPENDS
    include/*.cpp
    include/*/*.cpp
    include/*.h
    include/*/*.h
    include/*/*/*.h
)

add_library(Cragine STATIC ${ENGINE_SRC})

# Add spdlog include dir
target_include_directories(
    Cragine
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/deps/spdlog/include
    ${CMAKE_CURRENT_SOURCE_DIR}/deps/vma/include
    ${CMAKE_CURRENT_SOURCE_DIR}/deps/imgui
)

add_subdirectory(deps/wgpu)

# Link glfw to your Engine library
target_link_libraries(
    Cragine PUBLIC
    glfw
    webgpu
)
