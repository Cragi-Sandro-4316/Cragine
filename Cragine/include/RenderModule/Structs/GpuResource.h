#pragma once

#include "RenderModule/Structs/Buffer.h"
#include "RenderModule/Structs/ImageTexture.h"
#include "RenderModule/Structs/MeshCollection.h"
#include "RenderModule/Structs/TextureAtlas.h"
#include "RenderModule/Structs/Sampler.h"

namespace crg::renderer {
    struct GpuResource {

        enum Type {
            Buffer,
            Texture,
            MeshCollection,
            Atlas,
            Sampler
        };

       const Type type;

       Handle<renderer::Buffer> buffer;
       Handle<renderer::ImageTexture> texture;
       Handle<renderer::TextureAtlas> atlas;
       Handle<renderer::MeshCollection> meshCollection;
       Handle<renderer::Sampler> sampler;
    };
}

namespace crg {

    template<>
    struct Handle<renderer::GpuResource> {
        size_t id;
    };
}
