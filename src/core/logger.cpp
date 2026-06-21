#include "logger.h"
#include "spdlog/common.h"
#include <memory>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/spdlog.h>
#include <cstdlib>

namespace ne {
// TODO: NADER MAKE SURE logs/engine.log path is relative to the executable
// TODO: NADER Look the flush it seems that logs are not filled
// TODO: NADER Make sure everything compiles and runs fine in release

static std::shared_ptr<Logger> sEngineLogger;

Logger::Logger() {
  // Setup a colorful console sink
  auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();

  // Setup a file sink for persistent logs
  auto rotating_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>("build/logs/engine.log", 1024*1024, 5, true);

  // Combine sinks: log to both console and file
  spdlog::sinks_init_list sink_list = {console_sink, rotating_sink};

  mLogger = std::make_shared<spdlog::logger>("Engine", sink_list.begin(), sink_list.end());
  mLogger->set_level(spdlog::level::trace);
  mLogger->flush_on(spdlog::level::critical);
  // [Time] [Log Level] Message
  mLogger->set_pattern("[%T] [%^%l%$] %v");

  spdlog::register_logger(mLogger);
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
