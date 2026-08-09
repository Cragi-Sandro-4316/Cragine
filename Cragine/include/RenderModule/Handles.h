#pragma once
#include "Ecs/Ecs.h"

namespace crg {


    namespace renderer {
        struct Texture;
        struct TextureSampler;
        struct Buffer;
        struct Material;
        struct Mesh;
    }


    template<>
    struct Handle<renderer::Texture> {
        size_t id;
    };

    template<>
    struct Handle<renderer::TextureSampler> {
        size_t id;
    };

    template<>
    struct Handle<renderer::Mesh> {
        size_t id;
    };

    template<>
    struct Handle<renderer::Buffer> {
        size_t id;
    };

    template<>
    struct Handle<renderer::Material> {
        size_t id;
    };

}
