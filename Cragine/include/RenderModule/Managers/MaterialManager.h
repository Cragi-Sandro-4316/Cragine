#pragma once
#include "RenderModule/GpuInterface.h"
#include "RenderModule/Structs/Material.h"
#include "RenderModule/Structs/MaterialUpdate.h"
#include <unordered_map>

namespace crg::renderer {

    class GpuResourceManager;

    using MaterialID = size_t;

    class MaterialManager {
    public:

        Handle<Material> newMaterial(
            const std::filesystem::path& path,
            std::initializer_list<Handle<GpuResource>> res,
            GpuInterface& gpuInterface,
            MaterialUpdate::FuncType updateFunc = nullptr
        ) {

            Handle<Material> handle {
                .id = m_nextID++
            };

            m_materials.emplace(
                handle.id,
                Material(
                    path,
                    res,
                    gpuInterface
                )
            );

            if (updateFunc) {
                m_updates.emplace_back(
                    handle,
                    updateFunc
                );
            }

            return handle;
        }

        Material& getMaterial(Handle<Material> handle) {
            return m_materials.at(handle.id);
        }

        std::unordered_map<MaterialID, Material>& getMaterials() {
            return m_materials;
        }

        void runUpdates(GpuResourceManager& resManager) {
            for (auto& update : m_updates) {
                update.update(
                    m_materials.at(update.material.id),
                    resManager
                );
            }
        }

    private:

        MaterialID m_nextID = 0;

        std::unordered_map<MaterialID, Material> m_materials;
        std::vector<MaterialUpdate> m_updates;
    };


}
