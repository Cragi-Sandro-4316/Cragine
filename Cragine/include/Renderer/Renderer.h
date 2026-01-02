#pragma once

#include <webgpu/webgpu.h>


namespace crg::renderer {

    class Renderer {
    public:
        Renderer();
        ~Renderer();

    private:

        WGPUInstance m_instance{};
    };

}
