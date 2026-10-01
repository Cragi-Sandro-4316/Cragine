#pragma once

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
            m_materials.emplace(
                std::type_index(typeid(MaterialDef)),
                std::make_unique<Material<MaterialDef>>(
                    path,
                    renderContext,
                    def
                )
            );

            return (Material<MaterialDef>&)*m_materials.at(typeid(MaterialDef));
        }

        template<typename MaterialDef>
        Material<MaterialDef>& getMaterial() {
            return *static_cast<Material<MaterialDef>*>(m_materials.at(typeid(MaterialDef)));
        }

        std::unordered_map<
            std::type_index,
            std::unique_ptr<IMaterial>
        >& getMaterials() {
            return m_materials;
        }

    private:
        std::unordered_map<
            std::type_index,
            std::unique_ptr<IMaterial>
        > m_materials;
    };


}
