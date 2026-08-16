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



    struct IndexData {
        alignas(4) uint32_t vertexIdx;
        alignas(4) uint32_t instanceIdx;
    };

    struct MeshData {
        std::vector<VertexData> vertices;
        std::vector<IndexData> idxs;
    };


}
