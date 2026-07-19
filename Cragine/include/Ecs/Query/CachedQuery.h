#pragma once

#include <unordered_set>
#include <cstdint>

#include "Ecs/Archetypes/ComponentSignature.h"

namespace crg::ecs {


    class Archetype;
    class Chunk;

    class CachedQuery {
    public:

        CachedQuery(
            ComponentSignature signature,
            ComponentSignature withFilters,
            ComponentSignature withoutFilters,

            std::unordered_set<Archetype*>& archetypes
        );

        void update(Archetype* arch);

        std::unordered_set<Chunk*> m_chunks;
        uint32_t m_version = 0;
        ComponentSignature m_signature;
        ComponentSignature m_withFilters;
        ComponentSignature m_withoutFilters;

    };

}
