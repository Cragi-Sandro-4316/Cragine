#include "MeshServer.h"
#include "utils/Logger.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

namespace crg::renderer {

    void MeshServer::loadMeshFromObj(std::filesystem::path& path, Mesh& mesh) {

        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;

        std::string warn;
        std::string err;

        bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.string().c_str());

        if (!warn.empty()) {
            LOG_CORE_WARNING("Mesh loading warning: {}", warn);
        }

        if (!err.empty()) {
            LOG_CORE_WARNING("Mesh loading error: {}", err);
        }

        if (!ret) {
            return;
        }

        // Filling in vertexData:
        const auto& shape = shapes[0]; // look at the first shape only

        mesh.vertices.resize(shape.mesh.indices.size());


        // Filling in vertexData:
    	mesh.vertices.clear();
    	for (const auto& shape : shapes) {
    		size_t offset = mesh.vertices.size();
    		mesh.vertices.resize(offset + shape.mesh.indices.size());

    		for (size_t i = 0; i < shape.mesh.indices.size(); ++i) {
    			const tinyobj::index_t& idx = shape.mesh.indices[i];

    			mesh.vertices[offset + i].position = {
    				attrib.vertices[3 * idx.vertex_index + 0],
    				-attrib.vertices[3 * idx.vertex_index + 2], // Add a minus to avoid mirroring
    				// attrib.vertices[3 * idx.vertex_index + 1]
                    0
    			};

    			// Also apply the transform to normals!!
    			mesh.vertices[offset + i].normal = {
    				attrib.normals[3 * idx.normal_index + 0],
    				-attrib.normals[3 * idx.normal_index + 2],
    				attrib.normals[3 * idx.normal_index + 1]
    			};

    			mesh.vertices[offset + i].color = {
    				attrib.colors[3 * idx.vertex_index + 0],
    				attrib.colors[3 * idx.vertex_index + 1],
    				attrib.colors[3 * idx.vertex_index + 2]
    			};

                mesh.vertices[offset + i].uv = {
                    attrib.texcoords[2 * idx.texcoord_index + 0],
                    1 - attrib.texcoords[2 * idx.texcoord_index + 1]
                };
    		}
    	}

        LOG_CORE_INFO("Mesh {} loaded.", path.c_str());

        return;
    }


}
