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

    using Copy = void(*)(void* src, uint32_t srcIdx, void* dst, uint32_t dstIdx);

    using Write = void(*)(void* to, void* from);
    using ArrayWrite = void(*)(void* arr, void* data, size_t index);
    using ArraySwapNPop = void(*)(void* arr, size_t index, size_t count);
    using GetPtr = void*(*)(void* arr, size_t index);
    using EmitAdded = void(*)(EventManager& eventManager, Entity entity, void* data);
    using EmitRemoved = void(*)(EventManager& eventManager, Entity entity, void* data);

    struct FuncTable {
        Write write;
        ArrayWrite arrWrite;
        ArraySwapNPop swapAndPop;
        GetPtr getPtr;
        EmitAdded emitAdded;
        EmitRemoved emitRemoved;
    };
}
