project(Cragine)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Source files
file(GLOB_RECURSE ENGINE_SRC CMAKE_CONFIGURE_DEPENDS
    include/*.cpp
    include/*/*.cpp
    include/*.h
    include/*/*.h
    include/*/*/*.h
)

add_library(Cragine STATIC ${ENGINE_SRC})

# Link fmt to your Engine library
# target_link_libraries(Cragine PRIVATE spdlog)

# Link fmt to your Engine library
target_link_libraries(Cragine PRIVATE glfw)


# # Include directories
# target_include_directories(Engine PUBLIC
#     ${CMAKE_CURRENT_SOURCE_DIR}/src
#     ${CMAKE_CURRENT_SOURCE_DIR}/include
#     $<TARGET_PROPERTY:spdlog,INTERFACE_INCLUDE_DIRECTORIES>
#     $<TARGET_PROPERTY:glfw,INTERFACE_INCLUDE_DIRECTORIES>
# )
