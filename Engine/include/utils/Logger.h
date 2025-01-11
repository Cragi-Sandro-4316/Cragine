#pragma once
#include <spdlog/spdlog.h>

namespace Cragine {
    class Logger {
    public:
        static void init();

        inline static std::shared_ptr<spdlog::logger>& getCoreLogger() { return coreLogger; }
        inline static std::shared_ptr<spdlog::logger>& getClientLogger() { return clientLogger; }

    private:
        static std::shared_ptr<spdlog::logger> coreLogger;
        static std::shared_ptr<spdlog::logger> clientLogger;
    };
}

// Core Log Macro definitions
#define LOG_CORE_ERROR(...)     Cragine::Logger::getCoreLogger()->error(__VA_ARGS__)
#define LOG_CORE_WARNING(...)   Cragine::Logger::getCoreLogger()->warn(__VA_ARGS__)
#define LOG_CORE_INFO(...)      Cragine::Logger::getCoreLogger()->info(__VA_ARGS__)
#define LOG_CORE_TRACE(...)     Cragine::Logger::getCoreLogger()->trace(__VA_ARGS__)
#define LOG_CORE_FATAL(...)     Cragine::Logger::getCoreLogger()->error(__VA_ARGS__)

// Client Log Macro definitions
#define LOG_ERROR(...)          Cragine::Logger::getClientLogger()->error(__VA_ARGS__)
#define LOG_WARNING(...)        Cragine::Logger::getClientLogger()->warn(__VA_ARGS__)
#define LOG_INFO(...)           Cragine::Logger::getClientLogger()->info(__VA_ARGS__)
#define LOG_TRACE(...)          Cragine::Logger::getClientLogger()->trace(__VA_ARGS__)
#define LOG_FATAL(...)          Cragine::Logger::getCoreLogger()->error(__VA_ARGS__)