#pragma once

#include "RenderModule/Components/Mesh.h"
#include "glm/fwd.hpp"
#include <vector>
#include <webgpu/webgpu.hpp>


namespace crg::renderer {


    class MeshServer {
    public:

        std::pair<wgpu::Buffer, wgpu::BufferDescriptor> newTri(wgpu::Device& device, wgpu::Queue& queue) {

            std::vector<Vertex> vertices {
                Vertex {
                    .position = vec3(-0.75, -0.75, 0.),
                },
                Vertex {
                    .position = vec3(0.75, -0.75, 0.),
                },
                Vertex {
                    .position = vec3(0., 0.75, 0.),
                },
            };

            wgpu::BufferDescriptor buffDesc{};
            buffDesc.label = wgpu::StringView("Triangle akakakaka");
            buffDesc.size = sizeof(Vertex) * vertices.size();
            buffDesc.usage = wgpu::BufferUsage::Vertex | wgpu::BufferUsage::CopyDst;
            buffDesc.mappedAtCreation = false;


            wgpu::Buffer buffer = device.createBuffer(buffDesc);

            queue.writeBuffer(buffer, 0, vertices.data(), vertices.size() * sizeof(Vertex));
            return {buffer, buffDesc};
        }



    private:
        std::vector<Mesh> m_meshes;
    };


}
