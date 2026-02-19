#pragma once

#include "Rendererr/Components/MeshPool.h"
#include "Rendererr/Components/UniformData.h"
#include "Rendererr/utils/Loader.h"
#include "utils/Logger.h"
#include <vector>
#include <webgpu/webgpu.hpp>

namespace crg::renderer {

    struct ShaderData {
        std::vector<wgpu::Texture> textures;
        std::vector<wgpu::TextureView> textureViews;


        wgpu::ShaderModule shaderModule;

        wgpu::VertexBufferLayout vertLayout{};
        std::vector<wgpu::VertexAttribute> attributes;

        wgpu::FragmentState fragState{};
        wgpu::DepthStencilState depthStencilState{};
        wgpu::BlendState blendState{};
        wgpu::ColorTargetState colorTarget{};

        wgpu::BindGroupLayout bindGroupLayout;
    };

    // struct MaterialHandle {
    //     uint32_t index;
    // };

    class ShaderManager {
    public:

        ShaderData& addShader(
            wgpu::Device& device,
            wgpu::TextureFormat depthTextureFormat,
            wgpu::TextureFormat surfaceFormat
        ) const {
            m_shaders.emplace_back(ShaderData{});
            auto& shader = m_shaders.back();

            shader.shaderModule = Loader::loadShader(RESOURCE_DIR "/shader.wgsl", device);

            shader.attributes.resize(4);

            shader.attributes[0].shaderLocation = 0;
            shader.attributes[0].format = wgpu::VertexFormat::Float32x3;
            shader.attributes[0].offset = offsetof(VertexAttributes, position);

            // Corresponds to @location(1) normal
            shader.attributes[1].shaderLocation = 1;
            shader.attributes[1].format = wgpu::VertexFormat::Float32x3;
            shader.attributes[1].offset = offsetof(VertexAttributes, normal);

            // Corresponds to @location(2) color
            shader.attributes[2].shaderLocation = 2;
            shader.attributes[2].format = wgpu::VertexFormat::Float32x3;
            shader.attributes[2].offset = offsetof(VertexAttributes, color);

            // Corresponds to @location(3) uv
            shader.attributes[3].shaderLocation = 3;
            shader.attributes[3].format = wgpu::VertexFormat::Float32x2;
            shader.attributes[3].offset = offsetof(VertexAttributes, uv);

            shader.vertLayout.attributeCount = static_cast<uint32_t>(shader.attributes.size());
            shader.vertLayout.attributes = shader.attributes.data();
            shader.vertLayout.arrayStride = sizeof(VertexAttributes);
            shader.vertLayout.stepMode = wgpu::VertexStepMode::Vertex;


            LOG_CORE_INFO("Depth texture format: {}", (int)depthTextureFormat);
            shader.depthStencilState.format = depthTextureFormat;
            shader.depthStencilState.depthWriteEnabled = WGPUOptionalBool_True;
            shader.depthStencilState.depthCompare = wgpu::CompareFunction::Less;

            shader.blendState.color.srcFactor = wgpu::BlendFactor::SrcAlpha;
            shader.blendState.color.dstFactor = wgpu::BlendFactor::OneMinusSrcAlpha;
            shader.blendState.color.operation = wgpu::BlendOperation::Add;
            shader.blendState.alpha.srcFactor = wgpu::BlendFactor::Zero;
            shader.blendState.alpha.dstFactor = wgpu::BlendFactor::One;
            shader.blendState.alpha.operation = wgpu::BlendOperation::Add;

            shader.colorTarget.format = surfaceFormat;
            shader.colorTarget.blend = &shader.blendState;
            shader.colorTarget.writeMask = wgpu::ColorWriteMask::All;

            shader.fragState.module = shader.shaderModule;
            shader.fragState.entryPoint = wgpu::StringView("fs_main");
            shader.fragState.constantCount = 0;
            shader.fragState.constants = nullptr;
            shader.fragState.targetCount = 1;
            shader.fragState.targets = &shader.colorTarget;

            shader.bindGroupLayout = getBindGroupLayout(device);

            // TODO: Load texture, make view and sampler

            shader.textureViews.emplace_back(wgpu::TextureView{});
            shader.textures.emplace_back(Loader::loadTexture(RESOURCE_DIR "/fourareen2K_albedo.jpg", device, &shader.textureViews.back()));

            return shader;
        }

        ShaderData& getShader(size_t index) const {
            return m_shaders[index];
        }

    private:
        mutable std::vector<ShaderData> m_shaders;

        wgpu::BindGroupLayout getBindGroupLayout(wgpu::Device& device) const {
            std::vector<wgpu::BindGroupLayoutEntry> materialEntries(3);
            auto& bindingLayout = materialEntries[0];
            bindingLayout.binding = 0;  // Binding(0) in wgsl
            bindingLayout.visibility = wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment;
            bindingLayout.buffer.type = wgpu::BufferBindingType::Uniform;
            bindingLayout.buffer.minBindingSize = sizeof(UniformData);
            bindingLayout.buffer.hasDynamicOffset = false;

            auto& textureBindingLayout = materialEntries[1];
            textureBindingLayout.binding = 1;
            textureBindingLayout.visibility = wgpu::ShaderStage::Fragment;
            textureBindingLayout.texture.sampleType = wgpu::TextureSampleType::Float;
            textureBindingLayout.texture.viewDimension = wgpu::TextureViewDimension::_2D;

            auto& samplerBindingLayout = materialEntries[2];
            samplerBindingLayout.binding = 2;
            samplerBindingLayout.visibility = wgpu::ShaderStage::Fragment;
            samplerBindingLayout.sampler.type = wgpu::SamplerBindingType::Filtering;


            wgpu::BindGroupLayoutDescriptor bindGroupLayoutDesc{};
            bindGroupLayoutDesc.entryCount = (uint32_t)materialEntries.size();
            bindGroupLayoutDesc.entries = materialEntries.data();

            return device.createBindGroupLayout(bindGroupLayoutDesc);
        }
    };

}
