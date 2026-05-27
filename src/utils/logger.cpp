#include "logger.h"
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include <cstdlib>

namespace ne {
// TODO: MAKE SURE logs/engine.log path is relative to the executable
// TODO: Look the flush it seems that logs are not filled
// TODO: Make sure everything compiles and runs fine in release
Logger::Logger() {
  // Setup a colorful console sink
  auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();

  // Setup a file sink for persistent logs
  auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(
      "logs/engine.log", true);

  // Combine sinks: log to both console and file
  spdlog::sinks_init_list sink_list = {console_sink, file_sink};

  mLogger = std::make_shared<spdlog::logger>("Engine", sink_list.begin(), sink_list.end());
  mLogger->set_level(spdlog::level::trace);
  // [Time] [Log Level] Message
  mLogger->set_pattern("[%T] [%^%l%$] %v");
}

Logger::~Logger() {
  mLogger->flush();
}

void Logger::logExec(LogType inLogType, std::string inLog) {
  switch (inLogType) {
    case LogType_Log: {
      mLogger->info(std::move(inLog));
      break;
    }
    case LogType_Warn: {
      mLogger->warn(std::move(inLog));
      break;
    }
    case LogType_Error: {
      mLogger->error(std::move(inLog));
      break;
    }
    case LogType_Fatal: {
      mLogger->critical(std::move(inLog));
      mLogger->flush();
      std::abort();
      break;
    }
  }
}

Logger& Logger::get() {
  static Logger instance;
  return instance;
}

} // namespace ne
