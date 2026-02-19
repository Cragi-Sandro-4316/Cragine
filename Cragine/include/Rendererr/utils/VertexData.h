#pragma once

#include "Rendererr/Components/MeshPool.h"
#include <cstdint>
#include <webgpu/webgpu.hpp>
#include <glm/glm.hpp>

namespace crg::renderer {


    class VertexBuffer {
    public:
        VertexBuffer(
            wgpu::Device& device,
            wgpu::Queue& queue,
            std::vector<VertexAttributes>  vertexData,
            wgpu::BufferUsage usage = wgpu::BufferUsage::Vertex | wgpu::BufferUsage::CopyDst
        ) :
        m_device(device),
        m_queue(queue),
        m_vertexData(vertexData) {

            // Create vertex buffer
            wgpu::BufferDescriptor vertDesc{};
            vertDesc.size = vertexData.size() * sizeof(VertexAttributes);
            vertDesc.usage = usage;
            vertDesc.mappedAtCreation = false;
            m_vertexBuffer = m_device.createBuffer(vertDesc);
            m_queue.writeBuffer(m_vertexBuffer, 0, vertexData.data(), vertDesc.size);


            // Create index buffer
            // wgpu::BufferDescriptor idxDesc{};
            // idxDesc.size = indexData.size() * sizeof(uint16_t);
            // idxDesc.size = (idxDesc.size + 3) & ~3; // Align to multiple of 4
            // idxDesc.usage = wgpu::BufferUsage::Index | wgpu::BufferUsage::CopyDst;
            // idxDesc.mappedAtCreation = false;
            // m_indexBuffer = m_device.createBuffer(idxDesc);
            // m_queue.writeBuffer(m_indexBuffer, 0, indexData.data(), idxDesc.size);


        }


        ~VertexBuffer() {
            release();
        }


        void release() {
            if (m_vertexBuffer) m_vertexBuffer.release();
            // if (m_indexBuffer) m_indexBuffer.release();
        }

        wgpu::Buffer vertexBuffer() const { return m_vertexBuffer; }
        // wgpu::Buffer indexBuffer() const { return m_indexBuffer; }
        // uint32_t indexCount() const { return m_indexCount; }
        // wgpu::VertexBufferLayout& layout() { return m_vertexLayout; }

        void updateVertexData(const std::vector<float>& vertexData) {
            m_queue.writeBuffer(m_vertexBuffer, 0, vertexData.data(), vertexData.size() * sizeof(float));
        }


        // void updateIndexData(const std::vector<float>& indexData) {
        //     size_t size = (indexData.size() * sizeof(uint16_t) + 3) & ~3;
        //     m_queue.writeBuffer(m_indexBuffer, 0, indexData.data(), size);
        // }

        std::vector<VertexAttributes>& getVertexData() {
            return m_vertexData;
        }

        // Releases all resources
        void terminate() {
            m_vertexBuffer.destroy();
            m_vertexBuffer.release();
            m_vertexData.clear();

        }

    private:
        wgpu::Buffer m_vertexBuffer;
        // wgpu::Buffer m_indexBuffer;


        wgpu::Queue& m_queue;
        wgpu::Device& m_device;



        std::vector<VertexAttributes> m_vertexData;

    };
}
