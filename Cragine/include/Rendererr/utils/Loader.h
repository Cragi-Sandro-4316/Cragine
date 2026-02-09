#pragma once

#include "filesystem"
#include "utils/Logger.h"
#include <fstream>
#include <webgpu/webgpu.hpp>

namespace crg::renderer {
    struct MeshData;

    class Loader {
    public:
        static void loadObj(const std::filesystem::path& path, MeshData& meshData);

        static wgpu::ShaderModule loadShader(
            const std::filesystem::path& path,
            wgpu::Device device
        ) {
            std::ifstream file(path);

            if (!file.is_open()) {
                LOG_CORE_ERROR("Failed to open file");
                return nullptr;
            }
            file.seekg(0, std::ios::end);
            size_t size = file.tellg();
            std::string shaderSource(size, ' ');
            file.seekg(0);
            file.read(shaderSource.data(), size);

            wgpu::ShaderSourceWGSL shaderCodeDesc{};
            shaderCodeDesc.chain.next = nullptr;
            shaderCodeDesc.chain.sType = wgpu::SType::ShaderSourceWGSL;
            shaderCodeDesc.code = wgpu::StringView(shaderSource.c_str());

            wgpu::ShaderModuleDescriptor shaderDesc{};
            shaderDesc.nextInChain = &shaderCodeDesc.chain;
            return device.createShaderModule(shaderDesc);
        }

    private:

    };
}
