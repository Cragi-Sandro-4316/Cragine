#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

using namespace glm;

namespace crg {

    struct Transform {
        vec3 translation = vec3(0);
        quat rotation;
        vec3 scale = vec3(1);

        mat4x4 toMatrix() {

            mat4 m = {
                scale.x,                    0,                      0,                  0,
                0,                          scale.y,                0,                  0,
                0,                          0,                      scale.z,            0,
                translation.x,              translation.y,          translation.z,      1
            };


            return m;
        }
    };


}
