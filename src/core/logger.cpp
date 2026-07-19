#include "core/logger.h"
#include "core/platform.h"

#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <filesystem>

namespace ne {

Logger::Logger() {
  // Setup a colorful console sink
  auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();

  // Setup a file sink for persistent logs
  std::filesystem::path log_path = std::filesystem::path(platform::getExecutableDirectory()) / "logs/engine.log";
  auto rotating_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(log_path.string(), 1024 * 1024, 5, true);

  // Combine sinks: log to both console and file
  spdlog::sinks_init_list sink_list = {console_sink, rotating_sink};

  mLogger = std::make_shared<spdlog::logger>("Engine", sink_list.begin(), sink_list.end());
  mLogger->set_level(spdlog::level::trace);
  mLogger->flush_on(spdlog::level::critical);
  // [Time] [Log Level] Message
  mLogger->set_pattern("[%T] [%^%l%$] %v");

  spdlog::register_logger(mLogger);
}

Logger::~Logger() { mLogger->flush(); }

void Logger::logExec(LogType iLogType, std::string iLog) {
  switch (iLogType) {
  case LogType_Log: {
    mLogger->info(std::move(iLog));
    break;
  }
  case LogType_Warn: {
    mLogger->warn(std::move(iLog));
    break;
  }
  case LogType_Error: {
    mLogger->error(std::move(iLog));
    break;
  }
  case LogType_Fatal: {
    mLogger->critical(std::move(iLog));
    mLogger->flush();
    break;
  }
  }
}

Logger& Logger::get() {
  static Logger instance;
  return instance;
}

} // namespace ne
