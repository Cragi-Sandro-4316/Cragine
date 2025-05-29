#include "Application.h"
#include <cstdio>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace Cragine {
    Application::Application() {}

    Application::~Application() {}

    void Application::run() {
        auto logger = spdlog::stdout_color_mt("Cragine");
        logger -> set_level(spdlog::level::trace);

        while(true) {
            logger->error("porco dio");
        }
    }
}