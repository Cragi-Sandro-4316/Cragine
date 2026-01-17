#include "Loader.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

namespace crg::renderer {
    bool ModelLoader::loadObjFile(const std::filesystem::path& path, std::vector<VertexAttributes>& vertexData) {
        tinyobj::attrib_t attrib;
    	std::vector<tinyobj::shape_t> shapes;
    	std::vector<tinyobj::material_t> materials;

    	std::string warn;
    	std::string err;

    	// Call the core loading procedure of TinyOBJLoader
    	bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.string().c_str());

    	// Check errors
    	if (!warn.empty()) {
    		std::cout << warn << std::endl;
    	}

    	if (!err.empty()) {
    		std::cerr << err << std::endl;
    	}

    	if (!ret) {
    		return false;
    	}

    	// Filling in vertexData:
    	vertexData.clear();
    	for (const auto& shape : shapes) {
    		size_t offset = vertexData.size();
    		vertexData.resize(offset + shape.mesh.indices.size());

    		for (size_t i = 0; i < shape.mesh.indices.size(); ++i) {
    			const tinyobj::index_t& idx = shape.mesh.indices[i];

    			vertexData[offset + i].position = {
    				attrib.vertices[3 * idx.vertex_index + 0],
    				-attrib.vertices[3 * idx.vertex_index + 2], // Add a minus to avoid mirroring
    				attrib.vertices[3 * idx.vertex_index + 1]
    			};

    			// Also apply the transform to normals!!
    			vertexData[offset + i].normal = {
    				attrib.normals[3 * idx.normal_index + 0],
    				-attrib.normals[3 * idx.normal_index + 2],
    				attrib.normals[3 * idx.normal_index + 1]
    			};

    			vertexData[offset + i].color = {
    				attrib.colors[3 * idx.vertex_index + 0],
    				attrib.colors[3 * idx.vertex_index + 1],
    				attrib.colors[3 * idx.vertex_index + 2]
    			};
    		}
    	}

    	return true;
    }
}
