#pragma once

#include <cstdint>
#include <functional>


namespace crg::ecs {

    class Archetype;
    class Chunk;

    struct Entity {
        uint32_t index;
        uint32_t generation;

        bool operator==(const Entity& other) const {
            return index == other.index && generation == other.generation;
        }
    };



    struct EntityLocation {
        Archetype* archetype;
        Chunk* chunk;
        uint32_t indexInChunk;
    };

    // struct Entity {
    //     EntityHandle handle;
    //     EntityLocation location;
    // };

}


namespace std {
    template<>
    struct hash<crg::ecs::Entity> {
        size_t operator()(const crg::ecs::Entity& entity) const noexcept {
            uint64_t value = (static_cast<uint64_t>(entity.generation) << 32) | entity.index;
            return std::hash<uint64_t>{}(value);

        }
    };
}
