#include "CachedQuery.h"
#include "Ecs/Archetypes/Archetype.h"
#include "Ecs/Archetypes/ComponentSignature.h"
#include "utils/Logger.h"

namespace crg::ecs {
    CachedQuery::CachedQuery(
        ComponentSignature signature,
        ComponentSignature withFilters,
        ComponentSignature withoutFilters,
        std::unordered_set<Archetype*>& archetypes
    ) :
    m_signature(signature),
    m_withFilters(withFilters),
    m_withoutFilters(withoutFilters) {
        for (auto& arch : archetypes) {

            for (auto& chunk : arch->getChunks()) {
                m_chunks.insert(chunk.get());
            }

        }

    }

    void CachedQuery::update(Archetype* arch) {
        for (auto& chunk : arch->getChunks()) {
            LOG_CORE_INFO("Inserted chunk with ent count: {}", chunk->m_entityCount);
            m_chunks.insert(chunk.get());
        }
        m_version++;
    }
}
