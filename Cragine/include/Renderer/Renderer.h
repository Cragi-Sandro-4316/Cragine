#pragma once

#include "Window.h"
#include <webgpu/webgpu.h>


namespace crg::renderer {

    class Renderer {
    public:
        Renderer(Window* window);
        ~Renderer();

        void update();

    private:
        void fetchInstance();

        void getSurface();

        void configureSurface();

        void fetchAdapter();

        void printAdapterInfo();

        void fetchDevice();

        void fetchQueue();

        std::pair<WGPUSurfaceTexture, WGPUTextureView> getNextSurfaceViewData();

    private:

        Window* m_window;

        WGPUInstance m_instance;
        WGPUSurface m_surface;
        WGPUDevice m_device;
        WGPUAdapter m_adapter;
        WGPUQueue m_queue;
    };

}
