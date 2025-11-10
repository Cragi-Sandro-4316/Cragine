#pragma once

#include "Ecs/Components/ComponentManager.h"
#include "Ecs/Components/ComponentSignature.h"
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
        void addEntity(const std::tuple<Components...>& data, Entity entityHandle) {

            if (!matchesArchetype<Components...>()) {
                LOG_CORE_ERROR("AddEntity Error: Type signature incorrect");
                return;
            }

            Chunk* chunk = findOrCreateChunk();

            copyTupleToChunk(chunk, data, entityHandle);
        }

        // Removes the given entity
        void removeEntity(Entity removeHandle);

        std::vector<Chunk>* getChunks() {
            return &m_chunks;
        }

        RawCompData getRawCompData(Entity entity) {
            RawCompData result{};
            result.buffers.reserve(m_types.size());

            for (auto& c : m_chunks) {
                if (c.entityCount <= 0) continue;

                auto it = c.entityIndices.find(entity);
                if (it == c.entityIndices.end()) continue;

                auto entityIndex = it->second;

                for (size_t i = 0; i < m_types.size(); i++) {
                    result.signatureIds[m_types[i]] = i;

                    auto compIdx = c.componentIndices.find(m_types[i].type);
                    if (compIdx == c.componentIndices.end()) {
                        LOG_CORE_ERROR("Raw data retrieval error: Component index not found in chunk for component {}", m_types[i].type.name());
                        return RawCompData{};
                    }

                    uint8_t* dataPtr = static_cast<uint8_t*>(c.componentBuffers[compIdx->second].get() + entityIndex * m_types[i].size);

                    result.buffers.emplace_back(dataPtr);
                }

                return result;
            }

            LOG_CORE_ERROR("Raw data retrieval error: entity chunk could not be found.");
            return RawCompData{};
        }


        void addFromRawData(RawCompData rawData, Entity entity) {
            for (auto& type : m_types) {
                if (!rawData.signatureIds.contains(type)) {
                    LOG_CORE_ERROR("Raw data error: Given data signature does not match this archetype.");
                    return;
                }
            }

            Chunk* chunk = findOrCreateChunk();

            for (auto& type : m_types) {

                auto idx = rawData.signatureIds[type];
                auto dataPtr = rawData.buffers[idx];

                auto it = chunk->componentIndices.find(type.type);
                if (it == chunk->componentIndices.end()) {
                    LOG_CORE_ERROR("Raw component copy error: Type not found in chunk ");
                    return;
                }

                uint8_t* buffer = chunk->componentBuffers[it->second].get();

                if (!type.copyFn) {
                    LOG_CORE_ERROR("Error: missing copy lambda for type {}", type.type.name());
                }

                type.copyFn(
                    buffer + chunk->entityCount * type.size,
                    dataPtr
                );


            }


            chunk->entities[chunk->entityCount] = entity;
            chunk->entityIndices[entity] = chunk->entityCount;
            chunk->entityCount++;
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

        Chunk* findOrCreateChunk();

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

            ComponentInfo* compInfo = nullptr;
            for (auto& t : m_types) {
                if (t.type == type) {
                    compInfo = &t;
                }
            }

            auto buffer = chunk->componentBuffers[it->second].get();

            compInfo->copyFn(
                buffer + entityIndex * sizeof(T),
                &component
            );
        }

    private:

        // Indices of types contained in this Archetype
        std::vector<ComponentInfo> m_types;

        // Vector of chunks
        std::vector<Chunk> m_chunks;

    };

}
