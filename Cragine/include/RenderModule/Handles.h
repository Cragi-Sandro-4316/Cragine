#pragma once
#include "Ecs/Ecs.h"
#include "RenderModule/Structs/MeshBuffer.h"

namespace crg {


    namespace renderer {
        struct Texture;
        struct TextureSampler;
        struct Buffer;
        struct Material;
        struct MeshData;
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
    struct Handle<renderer::MeshData> {
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
