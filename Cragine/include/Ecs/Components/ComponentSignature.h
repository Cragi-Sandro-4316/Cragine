#pragma once

#include "ComponentManager.h"
#include <cstddef>
#include <type_traits>
#include <vector>

namespace crg::ecs {
    using ComponentSignature = std::vector<ComponentInfo>;

    // Wrapper for raw data. Used in archetype move operations
    struct RawCompData {
        std::vector<uint8_t*> buffers;
        std::unordered_map<ComponentInfo, size_t> signatureIds;
    };


    template<typename Component>
    struct With{ using type = Component; };

    template<typename Component>
    struct is_with : std::false_type {};
    template<typename Component>
    struct is_with<With<Component>> : std::true_type {};

    template<typename Component>
    struct Without{ using type = Component; };

    template<typename Component>
    struct is_without : std::false_type {};
    template<typename Component>
    struct is_without<Without<Component>> : std::true_type {};

    template <typename... Ts>
    struct filter_all;

    template <typename... Ts>
    struct filter_all<std::tuple<Ts...>> {
        using type = decltype(std::tuple_cat(
            std::conditional_t<
                !is_with<Ts>::value && !is_without<Ts>::value,
                std::tuple<Ts>,
                std::tuple<>
            >{}...
        ));
    };

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
