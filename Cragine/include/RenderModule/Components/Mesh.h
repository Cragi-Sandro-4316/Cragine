#pragma once

#include <glm/glm.hpp>

using namespace glm;

namespace crg::renderer {

    struct Vertex {
        vec4 position;
        vec4 color;
    };

    struct Index {
        uint32_t index;
    };

    struct Mesh {
        std::vector<Vertex> vertices;
    };


}
