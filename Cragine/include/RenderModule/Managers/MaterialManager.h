#pragma once

#include "Ecs/Handle.h"

#include "RenderModule/Structs/Material.h"
#include <typeindex>

namespace crg::renderer {

    class MaterialManager {
    public:

        template<typename MaterialDef>
        Material<MaterialDef>& newMaterial(
            const std::filesystem::path path,
            RenderContext& renderContext,
            MaterialDef def
        ) {

            auto id = std::hash<std::type_index>{}(typeid(MaterialDef));

            m_materials.emplace(
                id,
                std::make_unique<Material<MaterialDef>>(
                    path,
                    renderContext,
                    def
                )
            );

            return (Material<MaterialDef>&)*m_materials.at(id);
        }

        template<typename MaterialDef>
        Material<MaterialDef>& getMaterial() {
            auto id = std::hash<std::type_index>{}(typeid(MaterialDef));

            return (Material<MaterialDef>&)*m_materials.at(id);
        }

        template<typename MaterialDef>
        Handle<IMaterial> getHandle() {
            return Handle<IMaterial> {
                .id = std::hash<std::type_index>{}(typeid(MaterialDef))
            };
        }

        IMaterial& getMaterial(Handle<IMaterial> handle) {
            return *m_materials.at(handle.id);
        }


        std::unordered_map<
            size_t,
            std::unique_ptr<IMaterial>
        >& getMaterials() {
            return m_materials;
        }

    private:
        std::unordered_map<
            size_t,
            std::unique_ptr<IMaterial>
        > m_materials;
    };


}
