#pragma once

#include "Ecs/Entity/Entity.h"
#include "Ecs/Events/EventManager.h"
#include <algorithm>
#include <cstdint>
#include <unordered_set>
#include <vector>

namespace crg::ecs {


    using ComponentID = uint32_t;
    using ComponentSignature = std::unordered_set<ComponentID>;
    // Custom hash function for ComponentSignature
    struct ComponentSignatureHash {
        std::size_t operator()(const ComponentSignature& signature) const {

            std::vector<ComponentID> sorted(signature.begin(), signature.end());
            std::sort(sorted.begin(), sorted.end());

            std::size_t seed = 0;
            for (ComponentID id : sorted) {
                seed ^= std::hash<ComponentID>{}(id) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            }
            return seed;
        }
    };



    struct FuncTable {
        void (*remove)(void* arr, size_t index, size_t count);
        void* (*getPtr)(void* arr, size_t index);
        void (*emitRemoved)(void* arr, size_t index, EventManager& eventManager, Entity& entity);

    };
}
