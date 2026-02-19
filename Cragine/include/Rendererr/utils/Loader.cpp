#include "./Loader.h"
#include "../Components/MeshPool.h"
#include "utils/Logger.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

namespace crg::renderer {
    void Loader::loadObj(const std::filesystem::path& path, MeshData& meshData) {
    	LOG_CORE_ERROR("path: {}", path.c_str());
        tinyobj::attrib_t attrib;
    	std::vector<tinyobj::shape_t> shapes;
    	std::vector<tinyobj::material_t> materials;

    	std::string warn;
    	std::string err;

    	// Call the core loading procedure of TinyOBJLoader
    	bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.string().c_str());

    	// Check errors
    	if (!warn.empty()) {
       		LOG_CORE_WARNING("Mesh loading warning: {}", warn);

    	}

    	if (!err.empty()) {
       		LOG_CORE_ERROR("Mesh loading error: {}", err);
    	}

    	if (!ret) {
       		LOG_CORE_ERROR("Mesh loading error: something went wrong");
    		return;
    	}

        meshData.attributes.clear();
        for (const auto& shape : shapes) {
            size_t offset = meshData.attributes.size();
            meshData.attributes.resize(offset + shape.mesh.indices.size());

            for (size_t i = 0; i < shape.mesh.indices.size(); i++) {
                const tinyobj::index_t& idx = shape.mesh.indices[i];

                meshData.attributes[offset + i].position = {
                    attrib.vertices[3 * idx.vertex_index + 0],
    				-attrib.vertices[3 * idx.vertex_index + 2], // Add a minus to avoid mirroring
    				attrib.vertices[3 * idx.vertex_index + 1]
                };

                // Also apply the transform to normals!!
     			meshData.attributes[offset + i].normal = {
        				attrib.normals[3 * idx.normal_index + 0],
        				-attrib.normals[3 * idx.normal_index + 2],
        				attrib.normals[3 * idx.normal_index + 1]
     			};

     			meshData.attributes[offset + i].color = {
        				attrib.colors[3 * idx.vertex_index + 0],
        				attrib.colors[3 * idx.vertex_index + 1],
        				attrib.colors[3 * idx.vertex_index + 2]
     			};

                meshData.attributes[offset + i].uv = {
                    attrib.texcoords[2 * idx.texcoord_index + 0],
                    1 - attrib.texcoords[2 * idx.texcoord_index + 1]
                };
            }
        }

    }

    wgpu::Texture Loader::loadTexture(
        const std::filesystem::path& path,
        wgpu::Device device,
        wgpu::TextureView* pTextureView
    ) {
        int width = 0;
        int height = 0;
        int channels = 0;
        uint8_t* pixelData = stbi_load(path.string().c_str(), &width, &height, &channels, 4);

        if (!pixelData) return nullptr;

        wgpu::TextureDescriptor textureDesc;

        textureDesc.dimension = wgpu::TextureDimension::_2D;
        textureDesc.format = wgpu::TextureFormat::RGBA8Unorm; // by convention for bmp, png and jpg file. Be careful with other formats.
        textureDesc.sampleCount = 1;
        textureDesc.size = { (unsigned int)width, (unsigned int)height, 1 };
        textureDesc.usage = wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::CopyDst;
        textureDesc.viewFormatCount = 0;
        textureDesc.viewFormats = nullptr;
        textureDesc.mipLevelCount = std::bit_width(std::max(textureDesc.size.width, textureDesc.size.height));

        wgpu::Texture texture = device.createTexture(textureDesc);

        writeMipMaps(device, texture, textureDesc.size, textureDesc.mipLevelCount, pixelData);

        stbi_image_free(pixelData);

        if (pTextureView) {
            wgpu::TextureViewDescriptor textureViewDesc;
            textureViewDesc.aspect = wgpu::TextureAspect::All;
            textureViewDesc.baseArrayLayer = 0;
            textureViewDesc.arrayLayerCount = 1;
            textureViewDesc.baseMipLevel = 0;
            textureViewDesc.mipLevelCount = textureDesc.mipLevelCount;
            textureViewDesc.dimension = wgpu::TextureViewDimension::_2D;
            textureViewDesc.format = textureDesc.format;
            *pTextureView = texture.createView(textureViewDesc);
        }

        return texture;
    }

    void Loader::writeMipMaps(
        wgpu::Device device,
        wgpu::Texture texture,
        wgpu::Extent3D textureSize,
        [[maybe_unused]] uint32_t mipLevelCount, // not used yet
        const unsigned char* pixelData
    ) {

        auto queue = device.getQueue();
        wgpu::TexelCopyTextureInfo destination;
        destination.texture = texture;
        destination.origin = { 0, 0, 0 };
        destination.aspect = wgpu::TextureAspect::All;

        wgpu::TexelCopyBufferLayout source;
        source.offset = 0;

        // Create image data
        wgpu::Extent3D mipLevelSize = textureSize;
        std::vector<unsigned char> previousLevelPixels;
        wgpu::Extent3D previousMipLevelSize;
        for (uint32_t level = 0; level < mipLevelCount; ++level) {
            // Pixel data for the current level
            std::vector<unsigned char> pixels(4 * mipLevelSize.width * mipLevelSize.height);
            if (level == 0) {
                // We cannot really avoid this copy since we need this
                // in previousLevelPixels at the next iteration
                memcpy(pixels.data(), pixelData, pixels.size());
            }
            else {
                // Create mip level data
                for (uint32_t i = 0; i < mipLevelSize.width; ++i) {
                    for (uint32_t j = 0; j < mipLevelSize.height; ++j) {
                        unsigned char* p = &pixels[4 * (j * mipLevelSize.width + i)];
                        // Get the corresponding 4 pixels from the previous level
                        unsigned char* p00 = &previousLevelPixels[4 * ((2 * j + 0) * previousMipLevelSize.width + (2 * i + 0))];
                        unsigned char* p01 = &previousLevelPixels[4 * ((2 * j + 0) * previousMipLevelSize.width + (2 * i + 1))];
                        unsigned char* p10 = &previousLevelPixels[4 * ((2 * j + 1) * previousMipLevelSize.width + (2 * i + 0))];
                        unsigned char* p11 = &previousLevelPixels[4 * ((2 * j + 1) * previousMipLevelSize.width + (2 * i + 1))];
                        // Average
                        p[0] = (p00[0] + p01[0] + p10[0] + p11[0]) / 4;
                        p[1] = (p00[1] + p01[1] + p10[1] + p11[1]) / 4;
                        p[2] = (p00[2] + p01[2] + p10[2] + p11[2]) / 4;
                        p[3] = (p00[3] + p01[3] + p10[3] + p11[3]) / 4;
                    }
                }
            }

            // Upload data to the GPU texture
            destination.mipLevel = level;
            source.bytesPerRow = 4 * mipLevelSize.width;
            source.rowsPerImage = mipLevelSize.height;
            queue.writeTexture(destination, pixels.data(), pixels.size(), source, mipLevelSize);

            previousLevelPixels = std::move(pixels);
            previousMipLevelSize = mipLevelSize;
            mipLevelSize.width /= 2;
            mipLevelSize.height /= 2;
        }
        queue.release();
    }

}
