project(Cragine)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Add spdlog include dir
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/deps/spdlog/include)

# Source files
file(GLOB_RECURSE ENGINE_SRC CMAKE_CONFIGURE_DEPENDS
    include/*.cpp
    include/*/*.cpp
    include/*.h
    include/*/*.h
    include/*/*/*.h
)

add_library(Cragine STATIC ${ENGINE_SRC})

# Link glfw to your Engine library
target_link_libraries(Cragine PRIVATE glfw)


