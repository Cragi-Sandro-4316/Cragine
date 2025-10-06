#include "Logger.h"

namespace crg {
    // std::shared_ptr<spdlog::logger> Logger::m_coreLogger;
    // std::shared_ptr<spdlog::logger> Logger::m_clientLogger;

    void Logger::init() {
        // sets the color of the message and the following logging pattern:
        // Timestamp, Logger name, Message
        spdlog::set_pattern("%^[%T] %n: %v%$");

        m_coreLogger = spdlog::stdout_color_mt("Cragine");
        m_coreLogger -> set_level(spdlog::level::trace);

        m_clientLogger = spdlog::stdout_color_mt("Application");
        m_clientLogger -> set_level(spdlog::level::trace);

    }
}
