#pragma once

#include <format>
#include <memory>
#include <string>

namespace spdlog {
class logger;
}

namespace ne {

class Logger {
public:
  enum LogType { LogType_Log, LogType_Warn, LogType_Error, LogType_Fatal };

  static Logger &get();

  Logger();
  ~Logger();

  template <typename... Args>
  void log(LogType inLogType, const std::format_string<Args...> fmt, Args &&...args) {
    std::string formatted = std::vformat(fmt.get(), std::make_format_args(args...));
    logExec(inLogType, std::move(formatted));
  }

private:
  void logExec(LogType inLogType, std::string inLog);
  std::shared_ptr<spdlog::logger> mLogger;
};

} // namespace ne

#if !NE_BUILD_SHIPPING
#define NE_LOG(...) ne::Logger::get().log(ne::Logger::LogType_Log, __VA_ARGS__)
#define NE_WARN(...)                                                           \
  ne::Logger::get().log(ne::Logger::LogType_Warn, __VA_ARGS__)
#define NE_ERROR(...)                                                          \
  ne::Logger::get().log(ne::Logger::LogType_Error, __VA_ARGS__)
#define NE_FATAL(...)                                                          \
  ne::Logger::get().log(ne::Logger::LogType_Fatal, __VA_ARGS__)
#else
// Stripped clean out of production builds
#define NE_LOG(...)
#define NE_WARN(...)
#define NE_ERROR(...)
#define NE_FATAL(...)
#endif // NE_BUILD_SHIPPING
