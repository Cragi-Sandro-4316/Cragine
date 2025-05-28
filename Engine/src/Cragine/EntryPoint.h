#pragma once
#include "Application.h"
#include <cstdio>

extern Cragine::Application* Cragine::createApplication();

int main(int argc, char** argv) {

    auto app = Cragine::createApplication();
    app->run();
    delete app;

}