#pragma once

#include "Ecs/Handle.h"
#include <cstddef>
#include <type_traits>

namespace crg {

    class Mesh;
    namespace renderer {
        class Buffer;
        class CameraBuffer;
        class GpuResource;
        class ImageTexture;
        class MeshCollection;
        class Sampler;
        class TextureAtlas;
        class AtlasEntry;
    }

    template<>
    struct Handle<renderer::Buffer> {
        size_t id = -1;
    };

    template<typename T> struct is_gpuResource : std::false_type {};
    template<> struct is_gpuResource<renderer::Buffer> : std::true_type {};

    template<>
    struct Handle<renderer::CameraBuffer> {
        size_t id = -1;
    };

    template<> struct is_gpuResource<renderer::CameraBuffer> : std::true_type {};


    template<>
    struct Handle<renderer::GpuResource> {
        size_t id = -1;
    };

    template<>
    struct Handle<renderer::ImageTexture> {
        size_t id = -1;
    };

    template<> struct is_gpuResource<renderer::ImageTexture> : std::true_type {};


    template<>
    struct Handle<renderer::MeshCollection> {
        size_t id = -1;
    };

    template<> struct is_gpuResource<renderer::MeshCollection> : std::true_type {};


    template<>
    struct Handle<renderer::Sampler> {
        size_t id = -1;
    };

    template<> struct is_gpuResource<renderer::Sampler> : std::true_type {};


    template<>
    struct Handle<renderer::TextureAtlas> {
        size_t id = -1;
    };

    template<> struct is_gpuResource<renderer::TextureAtlas> : std::true_type {};

    template<>
    struct Handle<renderer::AtlasEntry> {
        size_t id = -1;
    };

    template<>
    struct Handle<Mesh> {
        size_t id = -1;
        size_t instanceID = -1;
    };
}
