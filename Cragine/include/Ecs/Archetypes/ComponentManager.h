#pragma once

#include "ComponentSignature.h"
#include "Ecs/Entity/Entity.h"
#include "Ecs/Events/AddedEvent.h"
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

            m_funcs.emplace(componentID, FuncTable {
                .write = [](void* to, void* from) {
                    *(Component*)to = *(Component*)from;
                },
                .arrWrite = [](void* arr, void* data, size_t index) {
                    Component* buff = (Component*)arr;

                    buff[index] = *(Component*)data;
                },
                .swapAndPop = [](void* arr, size_t index, size_t count) {
                    Component* buff = (Component*)arr;

                    buff[index] = buff[count];
                },
                .getPtr = [](void* arr, size_t index) {
                    Component* buffer = (Component*)arr;

                    return (void*)&buffer[index];
                },
                .emitAdded = [](EventManager& eventManager, Entity e, void* data) {
                    eventManager.emit(Added<Component> {
                        .entity = e,
                        .component = *(Component*)data
                    });
                },
                .emitRemoved = [](EventManager& eventManager, Entity e, void* data) {
                    eventManager.emit(Removed<Component> {
                        .entity = e,
                        .component = *(Component*)data
                    });
                }
            });

            (*m_queryComponentMap)[componentID] = std::unordered_set<QueryID>();

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


        FuncTable getFuncTable(ComponentID compID) {
            return m_funcs[compID];
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

        std::unordered_map<ComponentID, const FuncTable> m_funcs;


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
