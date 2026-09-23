#pragma once
#include "glm/ext/quaternion_trigonometric.hpp"
#include "glm/fwd.hpp"
#include "glm/trigonometric.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace glm;

namespace crg {

    struct Transform {
        vec3 translation = vec3(0);
        quat rotation = quat(1, 0, 0, 0);
        vec3 scale = vec3(1);


        void rotate(float angle, vec3 axis) {
            if (axis.length() < 0.0001)
                return;

            quat rot = glm::angleAxis(radians(angle), normalize(axis));

            rotation *= rot;
        }

        void rotateRadians(float angle, vec3 axis) {
            if (axis.length() < 0.0001)
                return;

            quat rot = glm::angleAxis(angle, normalize(axis));

            rotation *= rot;
        }

        // void lookAt(vec3 target, vec3 up) {
        //     vec3 direction = target - translation;

        //     if (direction.length() < 0.0001)
        //         return;

        //     direction = normalize(direction);

        //     rotation = quatLookAt(direction, up);
        // }

        mat4x4 toMatrix() {

            mat4 modelMatrix{1.f};

            modelMatrix = glm::translate(modelMatrix, translation);

            modelMatrix *= mat4(normalize(rotation));

            modelMatrix = glm::scale(modelMatrix, scale);

            return modelMatrix;
        }
    };


}
