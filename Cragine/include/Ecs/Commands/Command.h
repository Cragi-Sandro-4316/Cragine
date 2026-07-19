#pragma once

#include <functional>

namespace crg::ecs {

    class World;
    struct Command {
        template<typename F>
        Command(F&& run) : m_run(std::forward<F>(run)) {}
        std::function<void(World&)> m_run;
    };

}
