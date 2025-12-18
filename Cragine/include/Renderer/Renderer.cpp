#include "Renderer.h"
#include "Renderer/Instance/Instance.h"
#include "utils/Logger.h"


namespace crg {

    Renderer::Renderer(GLFWwindow* window) :
    m_window(window) {
        LOG_CORE_INFO("Initialising renderer");

        m_instance = makeInstance("Cragine", m_deletionQueue);
    }

    Renderer::~Renderer() {
        LOG_CORE_INFO("Destroying renderer");
        while (m_deletionQueue.size() > 0) {
            m_deletionQueue.back()(m_instance);
            m_deletionQueue.pop_back();
        }
    }

}
