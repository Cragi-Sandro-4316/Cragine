#pragma once

#include "glm/fwd.hpp"
#include "glm/trigonometric.hpp"
#include "utils/Assert.h"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace glm;

namespace crg {


    struct CameraUniform {
        mat4 projectionMatrix{1.f};
    };


    struct Camera {

        void setOrthoProjection(
            float left, float right,
            float top, float bottom,
            float near, float far
        ) {
            m_uniform.projectionMatrix[0][0] = 2.f / (right - left);
            m_uniform.projectionMatrix[1][1] = 2.f / (bottom - top);
            m_uniform.projectionMatrix[2][2] = 1.f / (far - near);
            m_uniform.projectionMatrix[3][0] = -(right + left) / (right - left);
            m_uniform.projectionMatrix[3][1] = -(bottom + top) / (bottom - top);
            m_uniform.projectionMatrix[3][2] = -near / (far - near);
        }


        void setPerspectiveProjection(
            float FOVy,
            float aspect,
            float near,
            float far
        ) {

            auto fov = radians(FOVy);

            ASSERT(
                (glm::abs(aspect - std::numeric_limits<float>::epsilon()) > 0.0f),
                "Camera: aspect ratio: {} is not valid", aspect
            );

            const float tanHalfFovy = tan(fov / 2.f);
            m_uniform.projectionMatrix = glm::mat4{0.0f};
            m_uniform.projectionMatrix[0][0] = 1.f / (aspect * tanHalfFovy);
            m_uniform.projectionMatrix[1][1] = 1.f / (tanHalfFovy);
            m_uniform.projectionMatrix[2][2] = far / (far - near);
            m_uniform.projectionMatrix[2][3] = 1.f;
            m_uniform.projectionMatrix[3][2] = -(far * near) / (far - near);
        }

        const mat4 getProjection() const {
            return m_uniform.projectionMatrix;
        }

    private:
        CameraUniform m_uniform;
    };


}
