#pragma once

#include "Ecs/Archetypes/Archetype.h"
#include "Ecs/Archetypes/ComponentManager.h"
#include "Ecs/Archetypes/ComponentSignature.h"
#include <iterator>
#include <tuple>
#include <vector>

namespace crg::ecs {

    template<typename... Components>
    struct QueryIterator {
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = std::tuple<Components&...>;
        using pointer = value_type*;
        using reference = value_type;

        std::vector<Chunk*>& m_chunks;

        ComponentManager& m_componentManager;

        // Indexes the chunk
        size_t m_chunkIndex;

        // Indexes the entity inside the chunk
        size_t m_entityIndex;


        QueryIterator& operator++() {

            if (m_chunks.empty()) return *this;

            m_entityIndex++;
            if (
                m_entityIndex >= m_chunks[m_chunkIndex]->m_entityCount
            ) {
                // increase until non empty chunk
                do {
                    m_chunkIndex++;
                } while (
                    m_chunkIndex < m_chunks.size() - 1 &&
                    m_chunks[m_chunkIndex]->m_entityCount == 0
                );

                m_entityIndex = 0;
            }

            return *this;
        }

        reference operator*() const {
            std::tuple<Components&...> result = std::tie(getReference<Components>()...);

            return result;
        }

        bool operator==(const QueryIterator& other) const {
            return (
                m_chunkIndex == other.m_chunkIndex &&
                m_entityIndex == other.m_entityIndex
            );
        }

        bool operator!=(const QueryIterator& other) const {
            return !(*this == other);
        }


    private:


        template<typename Component>
        Component& getReference() const {
            ComponentID compID = m_componentManager.getID<Component>();

            Component* buffer = (Component*)m_chunks[m_chunkIndex]->getBuffer(compID);

            return buffer[m_entityIndex];
        }
    };

}
