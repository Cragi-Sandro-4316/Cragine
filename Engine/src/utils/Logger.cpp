#include "utils/Logger.h"

#include "spdlog/sinks/stdout_color_sinks-inl.h"

namespace Cragine {

    std::shared_ptr<spdlog::logger> Logger::coreLogger;
    std::shared_ptr<spdlog::logger> Logger::clientLogger;

    void Logger::init() {
        // sets the color of the message and the following logging pattern:
        // Timestamp, Logger name, Message
        spdlog::set_pattern("%^[%T] %n: %v%$");

        coreLogger = spdlog::stdout_color_mt("Cragine");
        coreLogger -> set_level(spdlog::level::trace);

        clientLogger = spdlog::stdout_color_mt("Application");
        clientLogger -> set_level(spdlog::level::trace);

    }

}
