#pragma once

#include <glm/ext/matrix_float4x4.hpp>

namespace crg::renderer {
    struct UniformData {
        glm::mat4x4 projectionMatrix;
        glm::mat4x4 viewMatrix;
        glm::mat4x4 modelMatrix;

        glm::vec4 color;
        float time;
        float _padding[3];
    };
}
