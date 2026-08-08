#pragma once

#include "glm/fwd.hpp"
#include <glm/glm.hpp>

using namespace glm;

namespace crg::renderer {

    struct VertexData {
        alignas(16) vec3 position;
        alignas(16) vec3 color;
        alignas(16) vec3 normal;
        alignas(8) vec2 uv;
    };


    struct Instance {
        alignas(4) uint32_t vertexCount;
        alignas(4) uint32_t indexCount;
        alignas(16) mat4 modelMatrix;
    };


    struct IndexData {
        alignas(4) uint32_t vertexIdx;
        alignas(4) uint32_t instanceIdx;
    };

    struct Mesh {
        std::vector<VertexData> vertices;
        std::vector<IndexData> idxs;
    };


}
