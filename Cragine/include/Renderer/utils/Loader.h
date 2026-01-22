#pragma once
#include "utils/Logger.h"
#include <vector>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <webgpu/webgpu.hpp>
#include "../BufferHelpers/VertexData.h"

#include <stb_image.h>

namespace crg::renderer {
    class ModelLoader {
    public:
        static bool loadGeometry(const std::filesystem::path& path, std::vector<float>& pointData, std::vector<uint16_t>& indexData, int dimensions) {
           	std::ifstream file(path);
           	if (!file.is_open()) {
          		return false;
           	}

           	pointData.clear();
           	indexData.clear();

           	enum class Section {
          		None,
          		Points,
          		Indices,
           	};
           	Section currentSection = Section::None;

           	float value;
           	uint16_t index;
           	std::string line;
           	while (!file.eof()) {
          		getline(file, line);
          		if (line == "[points]") {
         			currentSection = Section::Points;
          		}
          		else if (line == "[indices]") {
         			currentSection = Section::Indices;
          		}
          		else if (line[0] == '#' || line.empty()) {
         			// Do nothing, this is a comment
          		}
          		else if (currentSection == Section::Points) {
         			std::istringstream iss(line);
         			// Get x, y, r, g, b
         			for (int i = 0; i < dimensions + 3; ++i) {
        				iss >> value;
        				pointData.push_back(value);
         			}
          		}
          		else if (currentSection == Section::Indices) {
         			std::istringstream iss(line);
         			// Get corners #0 #1 and #2
         			for (int i = 0; i < 3; ++i) {
        				iss >> index;
        				indexData.push_back(index);
         			}
          		}
           	}
           	return true;
        }

        static bool loadObjFile(const std::filesystem::path& path, std::vector<VertexAttributes>& vertexData);

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

        static wgpu::Texture loadTexture(
            const std::filesystem::path& path,
            wgpu::Device device,
            wgpu::TextureView* pTextureView = nullptr
        );

        static void writeMipMaps(
            wgpu::Device device,
            wgpu::Texture texture,
            wgpu::Extent3D textureSize,
            [[maybe_unused]] uint32_t mipLevelCount, // not used yet
            const unsigned char* pixelData
        );

    };
}
