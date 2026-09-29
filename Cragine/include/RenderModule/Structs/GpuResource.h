#pragma once

#include "RenderModule/Handles.h"

namespace crg::renderer {
    struct GpuResource {

        enum Type {
            Buffer,
            Texture,
            MeshCollection,
            Atlas,
            Sampler,
            Camera
        };

       const Type type;

       Handle<renderer::Buffer> buffer;
       Handle<renderer::ImageTexture> texture;
       Handle<renderer::TextureAtlas> atlas;
       Handle<renderer::MeshCollection> meshCollection;
       Handle<renderer::Sampler> sampler;
       Handle<renderer::CameraBuffer> camera;

    };
}
