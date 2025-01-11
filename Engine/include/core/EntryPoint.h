#pragma once
#include "core/Application.h"
#include "utils/Logger.h"

extern Cragine::Application* createApplication();

int main(int argc, char** argv) {

    Cragine::Logger::init();

    LOG_CORE_WARNING("Initialized Core Logger");

    LOG_INFO("Initialized Client Logger");

    auto app = createApplication();
    app->run();
    delete app;

    return 0;
}