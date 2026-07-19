#pragma once

#include <cstddef>
#include "../Handle.h"
#include "spdlog/fmt/bundled/base.h"

namespace crg::ecs {

    using EntityId = size_t;
    using EntityGeneration = size_t;

    struct Entity {
        size_t id;
        size_t generation;
    };



    // template<>
    // struct Handle<Entity> {
    //     Entity entity;

    // };

}



template<>
struct fmt::formatter<crg::ecs::Entity> {
    // Presentation format: 's' for short, 'l' for long, etc.
    char presentation = 's'; // default format

    // Parse the format specifier (e.g., "{:s}" or "{:l}")
    constexpr auto parse(format_parse_context& ctx) {
        auto it = ctx.begin(), end = ctx.end();
        if (it != end && (*it == 's' || *it == 'l')) {
            presentation = *it++;
        }
        // Check for invalid format specifiers
        if (it != end && *it != '}') {
            // throw format_error("invalid format specifier for MyType");
        }
        return it;
    }

    // Format the object
    template <typename FormatContext>
    auto format(const crg::ecs::Entity& entity, FormatContext& ctx) const {
        if (presentation == 's') {
            return fmt::format_to(ctx.out(), "Entity[id: {}, generation: {}]", entity.id, entity.generation);
        } else if (presentation == 'l') {
            return fmt::format_to(ctx.out(), "Entity[id: {}, generation: {}]", entity.id, entity.generation);
        }
        // Default format
        return fmt::format_to(ctx.out(), "Entity[id: {}, generation: {}]", entity.id, entity.generation);
    }
};
