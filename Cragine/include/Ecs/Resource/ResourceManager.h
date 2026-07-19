#pragma once

#include "utils/Logger.h"
#include <memory>
#include <typeindex>
#include <unordered_map>


namespace crg {

    struct ResourceWrapper {
        std::shared_ptr<void> resource;
    };


    class ResourceManager {
    public:


        template<typename ResourceName>
        ResourceName* getResource() {
            const std::type_index resID = typeid(ResourceName);

            auto it = m_resources.find(resID);
            if (it == m_resources.end()) {
                LOG_CORE_WARNING("Resource not found");
                return nullptr;
            }

            ResourceName* res = static_cast<ResourceName*>(it->second.resource.get());
            return res;
        }


        template<typename ResourceName, typename... Args>
        void registerResource(Args&&... args) {

            m_resources[typeid(ResourceName)] = ResourceWrapper {
                std::make_shared<ResourceName>(args...)
            };

        }


    private:

        std::unordered_map<std::type_index, ResourceWrapper> m_resources;

    };


}
