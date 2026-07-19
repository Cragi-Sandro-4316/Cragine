#pragma once

#include <cstdint>
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

        std::tuple<
            std::vector<Components*>...
        >& m_buffers;

        std::vector<const uint32_t*>& m_chunkEntityCounts;

        // Indexes the chunk
        size_t m_chunkIndex;

        // Indexes the entity inside the chunk
        size_t m_entityIndex;

        QueryIterator& operator++() {

            if (m_chunkEntityCounts.empty()) return *this;

            m_entityIndex++;
            if (
                m_entityIndex >= *m_chunkEntityCounts[m_chunkIndex]
            ) {

                // increase until non empty chunk
                do {
                    m_chunkIndex++;
                    // LOG_CORE_INFO("chunks left: {}", chunksLeft);
                } while (
                    m_chunkIndex != m_chunkEntityCounts.size() &&
                    *m_chunkEntityCounts[m_chunkIndex] == 0
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

            auto& compBuffers = std::get<std::vector<Component*>>(m_buffers);

            Component* buffer = compBuffers[m_chunkIndex];

            return buffer[m_entityIndex];
        }


    };

}
