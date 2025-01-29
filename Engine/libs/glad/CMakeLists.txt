# Add glad source files
file(GLOB GLAD_SRC src/*.c)

# Create a static library for glad
add_library(glad STATIC ${GLAD_SRC})

# Include glad's public headers
target_include_directories(glad PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)