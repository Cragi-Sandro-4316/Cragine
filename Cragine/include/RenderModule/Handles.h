#pragma once
#include "Ecs/Ecs.h"


namespace crg::ecs {



    struct TextureAtlas{};
    struct Buffer{};


    template<>
    struct Handle<TextureAtlas> {

    };

    template<>
    struct Handle<Buffer> {

    };


}
