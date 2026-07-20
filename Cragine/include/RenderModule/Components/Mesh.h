#pragma once

#include <glm/glm.hpp>

using namespace glm;

namespace crg::renderer {

    struct Vertex {
        vec3 position;
        vec3 color;
        // float _pad;
    };



    struct Mesh {
        std::vector<Vertex> vertices;
    };


}
