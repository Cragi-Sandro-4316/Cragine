#pragma once

#include "ComponentSignature.h"
#include "Ecs/Query/QueryFilters.h"
#include <typeindex>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace crg::ecs {

    using QueryID = uint32_t;


    class ComponentManager {
    public:

        ComponentManager(
            std::unordered_map<
                ComponentID,
                std::unordered_set<QueryID>
            >* queryComponentMap
        ) : m_queryComponentMap(queryComponentMap) {}

        template<typename Component>
        void registerComponent() {
            if (m_componentIDs.contains(typeid(Component))) {
                return;
            }

            ComponentID componentID = m_nextID;
            m_nextID++;

            m_componentIDs[typeid(Component)] = componentID;
            m_componentTypeIDs.emplace(componentID, typeid(Component));
            m_componentSizes.emplace_back(sizeof(Component));
            m_componentAligns.emplace_back(alignof(Component));


            m_componentOverwrite[componentID] = [](void* buffer, size_t to, size_t from) {
                Component* compBuffer = (Component*)buffer;

                compBuffer[to] = compBuffer[from];
            };

            m_componentCopy[componentID] = [](void* src, uint32_t srcIdx, void* dst, uint32_t dstIdx) {
                Component* srcBuff = (Component*)src;
                Component* dstBuff = (Component*)dst;

                dstBuff[dstIdx] = srcBuff[srcIdx];
            };


            (*m_queryComponentMap)[componentID] = std::unordered_set<QueryID>();

        }


        ComponentOverwrite& getDeleter(ComponentID compID) {
            return m_componentOverwrite[compID];
        }

        ComponentCopy& getCopy(ComponentID compID) {
            return m_componentCopy[compID];
        }

        template<typename Component>
        inline ComponentID getID() {
            registerComponent<Component>();
            return m_componentIDs[typeid(Component)];
        }


        template<typename... Components>
        inline ComponentSignature getSignature() {
            ComponentSignature result = {getID<Components>() ...};
            return result;
        }

        template<typename... Components>
        std::tuple<
            ComponentSignature,
            ComponentSignature,
            ComponentSignature
        > unpackQueryTypes() {
            ComponentSignature requested{};
            ComponentSignature with{};
            ComponentSignature without{};

            (fillFilters<Components>(requested, with, without), ...);

            return {requested, with, without};
        }



        // Converts a component tuple into a type erased list
        template<typename... Components>
        inline std::vector<TypeErasedComponent> makeTypeErased(std::tuple<Components...>& componentData) {
            std::vector<TypeErasedComponent> result;

            result = {(extractData(std::get<Components>(componentData)), ...)};
            return result;
        }

        // Converts a component tuple into a type erased list
        template<typename Component>
        inline TypeErasedComponent makeSingleTypeErased(Component& componentData) {
            auto result = extractData(componentData);
            return result;
        }

        inline size_t getSize(ComponentID id) {
            return m_componentSizes[id];
        }


    private:

        ComponentID m_nextID = 0;

        // Maps a type_index to a componentID
        std::unordered_map<std::type_index, ComponentID> m_componentIDs;

        // Maps a componentID to its type_index
        std::unordered_map<ComponentID, std::type_index> m_componentTypeIDs;

        std::unordered_map<ComponentID, ComponentOverwrite> m_componentOverwrite;

        std::unordered_map<ComponentID, ComponentCopy> m_componentCopy;

        // List of all component sizes. Indexed by ComponentID
        std::vector<size_t> m_componentSizes;

        // List of all component aligns. Indexed by ComponentID
        std::vector<size_t> m_componentAligns;


        std::unordered_map<
            ComponentID,
            std::unordered_set<QueryID>
        >* m_queryComponentMap;


        template<typename Component>
        void fillFilters(ComponentSignature& requested, ComponentSignature& with, ComponentSignature& without) {

            if constexpr (is_with<Component>{}) {
                with.insert(getID<typename Component::type>());
            }
            else if constexpr(is_without<Component>{}) {
                without.insert(getID<typename Component::type>());
            }
            else {
                requested.insert(getID<Component>());
            }
        }

    };




}
