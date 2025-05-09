#pragma once
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/async.h> 

#include <memory>

namespace MPG
{

class Logger
{
public:

    inline Logger();

    inline void LogError(const std::string& message);
    inline void LogInfo(const std::string& message);
    inline void LogWarning(const std::string& message);
    inline void LogCritical(const std::string& message);

private:

    std::shared_ptr<spdlog::async_logger> logger;

};

inline Logger::Logger()
{
    spdlog::init_thread_pool(1024, 3);
    constexpr size_t max_file_size = 1048576;
    constexpr size_t max_files = 3;

    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    console_sink->set_pattern("[%T] %^%l:%$ %v");

    auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>("logs/log.txt", max_file_size, max_files);
    file_sink->set_pattern("[%Y-%m-%d %H:%M:%S] [%l] %v");

    std::vector<spdlog::sink_ptr> sinks{console_sink, file_sink};

    logger = std::make_shared<spdlog::async_logger>(
        "mpg_logger",
        sinks.begin(), sinks.end(),
        spdlog::thread_pool(),               
        spdlog::async_overflow_policy::block
    );

    spdlog::register_logger(logger);
}

inline void Logger::LogError(const std::string &message)
{
    logger->error(message);
}
inline void Logger::LogInfo(const std::string &message)
{
    logger->info(message);
}
inline void Logger::LogWarning(const std::string &message)
{
    logger->warn(message);
}
inline void Logger::LogCritical(const std::string &message)
{
    logger->critical(message);
}

}