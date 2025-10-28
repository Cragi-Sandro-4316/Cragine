#pragma once

#include "Ecs/Components/ComponentManager.h"
#include "Ecs/Entities/EntityManager.h"
#include "Ecs/Entities/Entity.h"
#include "utils/Logger.h"
#include <cstddef>
#include <cstdint>
#include <memory>
#include <typeindex>
#include <unordered_map>
#include <vector>

namespace crg::ecs {



    struct Chunk {
        const size_t MAX_CHUNK_CAPACITY = 64;

        // Vector of raw memory arrays (one per component)
        std::vector<std::unique_ptr<uint8_t[]>> componentBuffers;

        // Number of entities held in this chunk
        uint32_t entityCount = 0;

        // Index of a component in the buffer list
        std::unordered_map<std::type_index, size_t> componentIndices;

        // Maps entity handles to their respective index in the component arrays
        std::unordered_map<Entity, size_t> entityIndices;

        std::unordered_map<size_t, Entity> entities;
    };

    class Archetype {
    public:

        Archetype(std::vector<ComponentInfo>& infos) {
            m_chunks.push_back(Chunk{});

            std::sort(infos.begin(), infos.end(), [](const ComponentInfo& a, const ComponentInfo& b) {
                return a.type < b.type;
            });

            for (int i = 0; i < infos.size(); i++) {
                m_types.emplace_back(infos[i]);

                // Allocate memory buffers
                m_chunks.back()
                    .componentBuffers
                    .emplace_back(std::make_unique<uint8_t[]>(
                        infos[i].size * m_chunks.back().MAX_CHUNK_CAPACITY
                ));

                // Push the indices in the index map
                m_chunks.back().componentIndices[infos[i].type] = i;

            }

        }


        const std::vector<ComponentInfo> getTypes() const { return m_types; }

        template<typename... Components>
        void addEntity(const std::tuple<Components...>& data, ComponentRegistry* compRegistry, Entity entityHandle) {

            if (!matchesArchetype<Components...>()) {
                LOG_CORE_ERROR("AddEntity Error: Type signature incorrect");
                return;
            }

            Chunk* chunk = findOrCreateChunk(compRegistry);

            copyTupleToChunk(chunk, data, entityHandle);
        }

        // Removes the given entity
        void removeEntity(Entity removeHandle, EntityManager* entManager);

        std::vector<Chunk>* getChunks() {
            return &m_chunks;
        }

    private:
        template<typename... Components>
        bool matchesArchetype() const {
            if (sizeof...(Components) != m_types.size())
                return false;

            std::array<std::type_index, sizeof...(Components)> inputTypes = { typeid(Components)... };
            std::sort(inputTypes.begin(), inputTypes.end());

            for (size_t i = 0; i < inputTypes.size(); i++) {
                if (inputTypes[i] != m_types[i].type) return false;
            }

            return true;
        }

        Chunk* findOrCreateChunk(ComponentRegistry* registry);

        template<typename... Components>
        void copyTupleToChunk(Chunk* chunk, const std::tuple<Components...>& data, Entity entityHandle) {
            size_t entityIndex = chunk->entityCount;

            std::apply([&](
                const Components&... comps) {
                    ((copySingleComponent(chunk, comps, typeid(Components), entityIndex)), ...);
                },
                data
            );

            chunk->entities[chunk->entityCount] = entityHandle;
            chunk->entityIndices[entityHandle] = chunk->entityCount;
            chunk->entityCount++;
        }

        template<typename T>
        inline void copySingleComponent(Chunk* chunk, const T& component, const std::type_index type, size_t entityIndex) {
            auto it = chunk->componentIndices.find(type);
            if (it == chunk->componentIndices.end()) {
                LOG_CORE_ERROR("Component copy error: Type not found in signature");
                return;
            }

            auto buffer = chunk->componentBuffers[it->second].get();
            std::memcpy(buffer + entityIndex * sizeof(T), &component, sizeof(T));
        }

    private:

        // Indices of types contained in this Archetype
        std::vector<ComponentInfo> m_types;

        // Vector of chunks
        std::vector<Chunk> m_chunks;

    };

}
