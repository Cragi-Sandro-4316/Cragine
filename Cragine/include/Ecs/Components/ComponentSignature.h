#pragma once

#include "ComponentManager.h"
#include <cstddef>
#include <vector>

namespace crg::ecs {
    using ComponentSignature = std::vector<ComponentInfo>;

}

namespace std {
    template<>
    struct hash<crg::ecs::ComponentSignature> {
        size_t operator()(const crg::ecs::ComponentSignature& sig) const noexcept {
            size_t seed = 0;

            auto types = sig;

            std::sort(
                types.begin(),
                types.end(),
                [](const auto& a, const auto& b) {
                    return a.type < b.type;
                }
            );

            for (const auto& t : types) {
                seed ^= t.type.hash_code() + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            }
            return seed;
        }
    };
}
