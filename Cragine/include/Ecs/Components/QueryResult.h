#pragma once

#include "Ecs/Archetypes/Archetype.h"
#include <vector>

namespace crg::ecs {

    template<typename... Component>
    class QueryResult {
    public:
        QueryResult(std::vector<Chunk*> chunkLists) :
        m_chunkLists(std::move(chunkLists)) {}

        QueryResult() = default;

        struct Iterator;

        Iterator begin() {
            return Iterator(&m_chunkLists, 0, 0);
        }

        Iterator end() {
            return Iterator(&m_chunkLists, m_chunkLists.size(), 0);
        }

    private:

        std::vector<Chunk*> m_chunkLists;

    };


    // template<typename... Components>
    // struct QueryResult<Components...>::Iterator {
    //     using value_type = std::tuple<Components&...>;
    //     using reference = value_type;
    //     using iterator_category = std::forward_iterator_tag;

    //     const std::vector<std::vector<Chunk>*>* m_ChunkLists;
    //     size_t m_ListIndex;
    //     size_t m_ChunkIndex;
    //     size_t m_EntityIndex;

    //     Iterator(
    //         const std::vector<std::vector<Chunk>*>* chunkLists,
    //         size_t listIndex,
    //         size_t chunkIndex,
    //         size_t entityIndex
    //     ) :
    //     m_ChunkLists(chunkLists),
    //     m_ListIndex(listIndex),
    //     m_ChunkIndex(chunkIndex),
    //     m_EntityIndex(entityIndex) {}

    //     value_type operator*() const {
    //         assert(m_ListIndex < m_ChunkLists->size() && "Query error: Dereferencing end() iterator");
    //         // Extract current chunk
    //         const auto& chunk = (*(*m_ChunkLists)[m_ListIndex])[m_ChunkIndex];
    //         // Get references to the component arrays
    //         return getTupleFromChunk(chunk, m_EntityIndex, std::index_sequence_for<Components...>{});
    //     }

    //     Iterator& operator++() {
    //         if (m_ListIndex >= m_ChunkLists->size()) return *this; // Already at the end

    //         const auto& chunk = (*(*m_ChunkLists)[m_ListIndex])[m_ChunkIndex];
    //         ++m_EntityIndex;

    //         if (m_EntityIndex >= chunk.entityCount) {
    //             m_EntityIndex = 0;
    //             ++m_ChunkIndex;

    //             if (m_ChunkIndex >= (*(*m_ChunkLists)[m_ListIndex]).size()) {

    //                 m_ChunkIndex = 0;
    //                 ++m_ListIndex;
    //             }
    //         }
    //         return *this;
    //     }

    //     bool operator==(const Iterator& other) const {
    //         return m_ListIndex == other.m_ListIndex &&
    //                m_ChunkIndex == other.m_ChunkIndex &&
    //                m_EntityIndex == other.m_EntityIndex;
    //     }
    //     bool operator!=(const Iterator& other) const { return !(*this == other); }

    // private:
    //     template<std::size_t... Is>
    //     value_type getTupleFromChunk(const Chunk& chunk, size_t entityIndex, std::index_sequence<Is...>) const {
    //         return std::tie(getComponent<Components>(chunk, entityIndex)...);
    //     }

    //     // Retrieves component reference of the given entity index
    //     template<typename Component>
    //     Component& getComponent(const Chunk& chunk, size_t entityIndex) const {

    //         auto it = chunk.componentIndices.find(typeid(Component));
    //         assert(it != chunk.componentIndices.end() && "Query error: component not found in chunk!");

    //         auto compArr = reinterpret_cast<Component*>(chunk.componentBuffers[it->second].get());
    //         return compArr[entityIndex];
    //     }
    // };


    template<typename... Components>
    struct QueryResult<Components...>::Iterator {
        using value_type = std::tuple<Components&...>;
        using reference = value_type;
        using iterator_category = std::forward_iterator_tag;

        const std::vector<Chunk*>* m_ChunkLists;
        size_t m_ChunkIndex;
        size_t m_EntityIndex;

        Iterator(
            const std::vector<Chunk*>* chunkLists,
            size_t chunkIndex,
            size_t entityIndex
        ) :
            m_ChunkLists(chunkLists),
            m_ChunkIndex(chunkIndex),
            m_EntityIndex(entityIndex) {}

        value_type operator*() const {
            assert(m_ChunkIndex < m_ChunkLists->size() && "Query error: Dereferencing end() iterator");

            const Chunk& chunk = *(*m_ChunkLists)[m_ChunkIndex];
            return getTupleFromChunk(chunk, m_EntityIndex, std::index_sequence_for<Components...>{});
        }

        Iterator& operator++() {
            if (m_ChunkIndex >= m_ChunkLists->size()) return *this; // already end()

            const Chunk& chunk = *(*m_ChunkLists)[m_ChunkIndex];
            ++m_EntityIndex;

            if (m_EntityIndex >= chunk.entityCount) {
                m_EntityIndex = 0;
                ++m_ChunkIndex;
            }
            return *this;
        }

        bool operator==(const Iterator& other) const {
            return m_ChunkIndex == other.m_ChunkIndex &&
                   m_EntityIndex == other.m_EntityIndex;
        }
        bool operator!=(const Iterator& other) const { return !(*this == other); }

    private:
        template<std::size_t... Is>
        value_type getTupleFromChunk(const Chunk& chunk, size_t entityIndex, std::index_sequence<Is...>) const {
            return std::tie(getComponent<Components>(chunk, entityIndex)...);
        }

        template<typename Component>
        Component& getComponent(const Chunk& chunk, size_t entityIndex) const {
            auto it = chunk.componentIndices.find(typeid(Component));
            assert(it != chunk.componentIndices.end() && "Query error: component not found in chunk!");

            auto compArr = reinterpret_cast<Component*>(chunk.componentBuffers.at(it->second).get());
            return compArr[entityIndex];
        }
    };


}
