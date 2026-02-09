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



}
