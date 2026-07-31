#pragma once

#include <glm/glm.hpp>

using namespace glm;

namespace crg::renderer {

    struct Vertex {
        alignas(16) vec3 position;
        alignas(16) vec3 color;
        // alignas(16) vec3 normal;
        vec2 uv;
    };

    struct Index {
        uint32_t index;
    };

    struct Mesh {
        std::vector<Vertex> vertices;
    };


}
