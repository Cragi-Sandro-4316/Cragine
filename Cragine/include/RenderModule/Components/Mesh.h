#pragma once

#include <glm/glm.hpp>

using namespace glm;

namespace crg::renderer {

    struct Vertex {
        vec4 position;
        vec4 color;
    };

    struct Mesh {
        std::vector<Vertex> vertices;
    };


}
