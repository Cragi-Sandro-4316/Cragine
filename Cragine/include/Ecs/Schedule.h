#pragma once

#include <cstdint>
namespace crg::ecs {
    enum Schedule : uint8_t {
        Startup,
        FixedUpdate,
        Update
    };

}
