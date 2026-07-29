#pragma once
#include "Ecs/Ecs.h"
#include "RenderModule/Structs/Buffer.h"


namespace crg {

    struct TextureAtlas{};


    template<>
    struct Handle<TextureAtlas> {

    };

    template<>
    struct Handle<renderer::Buffer> {
        size_t id;
    };


}
