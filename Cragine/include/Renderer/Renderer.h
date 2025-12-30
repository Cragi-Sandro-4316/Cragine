#pragma once
#include "Device/Device.h"
#include "Instance/Instance.h"
#include "Renderer/utils/utils.h"
#include "Swapchain/Swapchain.h"
#include "Window.h"
#include <memory>
#include <vulkan/vulkan_core.h>

namespace crg::renderer {
    class Renderer {
    public:
        Renderer(Window* window);

        void draw();

    private:
        void initFrameData();

        FrameData& getCurrentFrame() { return m_frames[m_frameNumber % m_frames.size()]; };

        void initCommands();

        void initSyncStructs();

    private:

        uint32_t m_frameNumber = 0;

        Window* m_window = nullptr;

        std::unique_ptr<Instance> m_instance;
        std::unique_ptr<Device> m_device;

        std::unique_ptr<Swapchain> m_swapchain;

        VkExtent2D m_drawExtent;

        std::vector<FrameData> m_frames;

        AllocatedImage m_drawImage;

        utils::DeletionQueue m_deletionQueue;

    };
}
