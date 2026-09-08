#pragma once
#include "AssetManager/AssetManager.h"
#include "Ecs/Ecs.h"

namespace crg {


    namespace renderer {
        struct ImageTexture;
        struct TextureSampler;
        struct Buffer;
        struct Material;
        struct MeshData;
        struct Mesh;
        struct AtlasEntry;
        struct TextureAtlas;
    }


    template<>
    struct Handle<renderer::ImageTexture> {
        size_t id;
    };

    template<typename T> struct is_texture : std::false_type {};
    template<> struct is_texture<Handle<renderer::ImageTexture>> : std::true_type {};



    template<>
    struct Handle<renderer::TextureAtlas> {
        size_t id;
    };

    template<typename T> struct is_atlas : std::false_type {};
    template<> struct is_atlas<Handle<renderer::TextureAtlas>> : std::true_type {};


    template<>
    struct Handle<renderer::TextureSampler> {
        size_t id;
    };

    template<typename T> struct is_sampler : std::false_type {};
    template<> struct is_sampler<Handle<renderer::TextureSampler>> : std::true_type {};


    template<>
    struct Handle<renderer::MeshData> {
        size_t id;
    };

    template<>
    struct Handle<renderer::Mesh> {
        size_t id;
        size_t instanceId;
    };

    template<>
    struct Handle<renderer::Buffer> {
        size_t id;
    };

    template<typename T> struct is_buffer : std::false_type {};
    template<> struct is_buffer<Handle<renderer::Buffer>> : std::true_type {};


    template<>
    struct Handle<renderer::Material> {
        size_t id;
    };

    template<>
    struct Handle<renderer::AtlasEntry> {
        size_t id;
    };

}
