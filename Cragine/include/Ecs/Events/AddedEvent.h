#pragma once

#include "Ecs/Entity/Entity.h"

namespace crg::ecs {

    template<typename T>
    struct Added {
        Entity entity;
        T component;
    };

    template<typename T>
    struct Removed {
        Entity entity;
        T component;
    };

}
