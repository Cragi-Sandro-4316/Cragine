#pragma once
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <memory>

namespace crg {
    class Logger {
    public:
        static void init();

        inline static std::shared_ptr<spdlog::logger>& getCoreLogger() { return m_coreLogger; }
        inline static std::shared_ptr<spdlog::logger>& getClientLogger() { return m_clientLogger; }

    private:
        inline static std::shared_ptr<spdlog::logger> m_coreLogger;
        inline static std::shared_ptr<spdlog::logger> m_clientLogger;

    };

    // Core Log Macro definitions
    #define LOG_CORE_ERROR(...)     crg::Logger::getCoreLogger()->error(__VA_ARGS__)
    #define LOG_CORE_WARNING(...)   crg::Logger::getCoreLogger()->warn(__VA_ARGS__)
    #define LOG_CORE_INFO(...)      crg::Logger::getCoreLogger()->info(__VA_ARGS__)
    #define LOG_CORE_TRACE(...)     crg::Logger::getCoreLogger()->trace(__VA_ARGS__)
    #define LOG_CORE_FATAL(...)     crg::Logger::getCoreLogger()->error(__VA_ARGS__)

    // Client Log Macro definitions
    #define LOG_ERROR(...)          crg::Logger::getClientLogger()->error(__VA_ARGS__)
    #define LOG_WARNING(...)        crg::Logger::getClientLogger()->warn(__VA_ARGS__)
    #define LOG_INFO(...)           crg::Logger::getClientLogger()->info(__VA_ARGS__)
    #define LOG_TRACE(...)          crg::Logger::getClientLogger()->trace(__VA_ARGS__)
    #define LOG_FATAL(...)          crg::Logger::getCoreLogger()->error(__VA_ARGS__)
}
