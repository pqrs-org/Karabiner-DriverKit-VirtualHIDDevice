#pragma once

#include <filesystem>
#include <pqrs/spdlog.hpp>
#include <spdlog/async.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_sinks.h>

class logger final {
public:
  static std::shared_ptr<spdlog::logger> get_logger(void) {
    std::lock_guard<std::mutex> guard(mutex_);

    if (logger_) {
      return logger_;
    }

    //
    // Fallback
    //

    static std::shared_ptr<spdlog::logger> stdout_logger;
    if (!stdout_logger) {
      stdout_logger = pqrs::spdlog::factory::make_stdout_logger_mt("client");
    }

    return stdout_logger;
  }

  static void set_async_rotating_logger(const std::string& logger_name,
                                        const std::filesystem::path& log_file_path,
                                        const std::filesystem::perms& log_directory_perms) {
    auto l = pqrs::spdlog::factory::make_async_rotating_logger_mt(logger_name,
                                                                  log_file_path,
                                                                  log_directory_perms,
                                                                  256 * 1024,
                                                                  3);
    if (l) {
      l->flush_on(spdlog::level::info);
      l->set_pattern(pqrs::spdlog::get_pattern());

      std::lock_guard<std::mutex> guard(mutex_);
      logger_ = l;
    }
  }

private:
  static inline std::mutex mutex_;
  static inline std::shared_ptr<spdlog::logger> logger_;
};
