#pragma once

#include "utils/Logger.h"
#include <array>
#include <memory>
#include <typeindex>
#include <unordered_map>
#include <vector>


namespace crg {

    struct ResourceWrapper {
        std::shared_ptr<void> resource;
    };

    class ResourceManager {
    public:

        template<typename ResourceName>
        ResourceName& getResource() {
            static ResourceName EMPTY;
            std::type_index typeId = typeid(ResourceName);
            auto it = m_queues.find(typeId);
            if (it == m_queues.end()) {
               LOG_CORE_WARNING("Resource not found");
               return EMPTY;
            }

            ResourceName* resource = static_cast<ResourceName*>(it->second.resource.get());
            return *resource;
        }


        template<typename ResourceName>
        void newResource(ResourceName resource) {
            m_queues[typeid(ResourceName)] = ResourceWrapper {
                std::make_shared<ResourceName>(std::move(resource))
            };
        }

    private:
        std::unordered_map<std::type_index, ResourceWrapper> m_queues{};
    };

}
