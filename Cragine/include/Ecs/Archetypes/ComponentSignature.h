#pragma once

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


    using ComponentOverwrite = void(*)(void* buffer, size_t to, size_t from);
    using ComponentCopy = void(*)(void* src, uint32_t srcIdx, void* dst, uint32_t dstIdx);


    struct ComponentMetadata {
        size_t componentSize;
        ComponentID componentID;

        // TODO: Possible type erased constructor and destructor callbacks
        // TypeErasedCopy copyLambda;
        // TypeErasedDelete deleteLambda;
    };

    struct TypeErasedComponent {
        uint8_t* pData;
        ComponentMetadata metadata;
    };


}
