#pragma once

#include <deque>
#include <functional>

namespace crg::renderer::utils {
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
}
