#pragma once

#include "Ecs/Handle.h"

namespace crg {

    namespace renderer {
        struct IMaterial;
    }

    struct Mesh {
        size_t id;
        size_t instanceID;
        Handle<renderer::IMaterial> material;
    };

}
