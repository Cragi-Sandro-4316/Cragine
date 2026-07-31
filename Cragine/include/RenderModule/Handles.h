#pragma once
#include "Ecs/Ecs.h"
#include "RenderModule/Structs/Buffer.h"
#include "RenderModule/Structs/Texture.h"


namespace crg {


    template<>
    struct Handle<renderer::Texture> {
        size_t id;
    };

    template<>
    struct Handle<renderer::Buffer> {
        size_t id;
    };


}
