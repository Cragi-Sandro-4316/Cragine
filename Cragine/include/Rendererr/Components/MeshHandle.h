#pragma once

#include <cstdint>
#include <string>

namespace crg {
    struct MeshHandle {
        std::string path;
        uint32_t id;
        bool isLoaded = false;
    };
}
