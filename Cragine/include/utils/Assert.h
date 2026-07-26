#pragma once

#include "Logger.h"

#ifndef NDEBUG

#define ASSERT(condition, fmt, ...) \
if (!(condition)) {\
    LOG_CORE_ERROR("Assertion '" #condition "' failed \nin {} line {}. \n" fmt , __FILE__, __LINE__, __VA_ARGS__);\
    std::terminate(); \
}

#else
#define ASSERT(condition, message)
#endif
