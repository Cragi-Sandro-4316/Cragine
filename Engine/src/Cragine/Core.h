#pragma once

#include "Cragine/utils/Logger.h"

#define BIT(x) (1 << x)

#define DEBUG_BREAK \
    asm("int3");


#define CRAGINE_ASSERT(x, ...) { if (!x) { LOG_ERROR("Assertion Failed: {0}", __VA_ARGS__); DEBUG_BREAK } }