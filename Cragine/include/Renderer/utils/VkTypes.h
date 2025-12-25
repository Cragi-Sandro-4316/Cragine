#pragma once


#include "vk_mem_alloc.h"
#include "vulkan/vulkan.hpp"
#include <deque>
#include <functional>

namespace crg::renderer {

    struct DeletionQueue {
        std::deque<std::function<void()>> m_deletors;

        void pushFunction(std::function<void()>&& func) {
            m_deletors.push_back(func);
        }

        void flush() {
            for (auto it = m_deletors.rbegin(); it != m_deletors.rend(); it++) {
                (*it)();
            }

            m_deletors.clear();
        }
    };


    struct FrameData {
        vk::CommandPool m_commandPool;
        vk::CommandBuffer m_mainComandBuffer;

        vk::Semaphore m_swapchainSemaphore;
        vk::Semaphore m_renderSemaphore;
        vk::Fence m_renderFence;

        DeletionQueue m_deletionQueue;
    };


    struct AllocatedImage  {
        vk::Image image;
        vk::ImageView imageView;
        VmaAllocation allocation;
        vk::Extent3D imageExtent;
        vk::Format imageFormat;
    };


}
