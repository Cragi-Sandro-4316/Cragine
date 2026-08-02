#pragma once

#include "glm/fwd.hpp"
#include <glm/glm.hpp>

using namespace glm;

namespace crg::renderer {

    struct Vertex {
        alignas(16) vec3 position;
        alignas(16) vec3 color;
        alignas(16) vec3 normal;
        vec2 uv;
    };


    // struct Instance {
    //     alignas(4) uint32_t id;
    //     alignas(4) uint32_t vertexCount;
    //     alignas(4) uint32_t indexCount;
    //     alignas(16) mat4 modelMatrix;

    // };


    struct Index {
        uint32_t index;
    };

    struct Mesh {
        std::vector<Vertex> vertices;
        std::vector<Index> idxs;
    };


}
