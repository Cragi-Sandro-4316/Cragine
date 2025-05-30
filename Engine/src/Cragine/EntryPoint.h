#pragma once
#include "Application.h"
#include "utils/Logger.h"
#include <cstdio>

extern Cragine::Application* Cragine::createApplication();

int main(int argc, char** argv) {

    Cragine::Logger::init();

    LOG_CORE_WARNING("Initialized Core Logger");

    LOG_INFO("Initialized Client Logger");

    auto app = Cragine::createApplication();
    app->run();
    delete app;

}