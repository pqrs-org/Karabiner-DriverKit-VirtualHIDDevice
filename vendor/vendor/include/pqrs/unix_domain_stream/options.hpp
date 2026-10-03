#pragma once

// (C) Copyright Takayama Fumihiko 2026.
// Distributed under the Boost Software License, Version 1.0.
// (See https://www.boost.org/LICENSE_1_0.txt)

#include <chrono>
#include <cstddef>

namespace pqrs::unix_domain_stream {

namespace impl {
[[nodiscard]] inline std::chrono::milliseconds normalize_scheduling_interval(std::chrono::milliseconds value) noexcept {
  constexpr auto minimum_interval = std::chrono::milliseconds(100);

  if (value < minimum_interval) {
    return minimum_interval;
  }

  return value;
}
} // namespace impl

struct common_options {
  struct parameters final {
    // Soft limit for one application message payload.
    // This prevents excessive memory use when sending or receiving unexpectedly large frames.
    size_t max_message_size{32 * 1024};

    // Maximum number of unsent frames kept in the per-peer write queue.
    // This limits memory growth when the peer is slow or the caller sends faster
    // than the socket can write.
    size_t max_send_queue_size{1024};

    // Interval used to send heartbeat frames to the peer.
    std::chrono::milliseconds heartbeat_interval{3000};

    // Maximum idle time allowed without receiving any frame from the peer.
    // Heartbeat, health-check and user-data frames all refresh this deadline.
    std::chrono::milliseconds heartbeat_timeout{10000};

    // Maximum time allowed for one async read operation.
    std::chrono::milliseconds read_timeout{5000};

    // Maximum time allowed for one async write operation.
    std::chrono::milliseconds write_timeout{5000};

    // Whether request failures such as timeout should invalidate the current
    // connection. This is useful for request/response protocols where a failed
    // request means the stream state is no longer trustworthy.
    bool invalidate_connection_on_request_error{true};
  };

  common_options() = default;

  explicit common_options(const parameters& parameters)
      : common_parameters(parameters) {
  }

  parameters common_parameters;
};

struct client_options final : public common_options {
  struct parameters final {
    // Interval used to retry client connect after a failure.
    std::chrono::milliseconds reconnect_interval{1000};
  };

  client_options() = default;
  explicit client_options(const common_options::parameters& common_parameters);
  client_options(const common_options::parameters& common_parameters,
                 const parameters& parameters);

  parameters client_parameters;
};

struct server_options final : public common_options {
  struct parameters final {
    // Interval used to retry server bind after a failure.
    std::chrono::milliseconds bind_retry_interval{1000};

    // Interval used by the server to verify that its socket path is still
    // connectable and that the accepted connection can exchange an internal
    // health-check frame.
    std::chrono::milliseconds socket_path_health_check_interval{3000};

    // Maximum time allowed for one socket path health check.
    std::chrono::milliseconds socket_path_health_check_timeout{1000};
  };

  server_options() = default;
  explicit server_options(const common_options::parameters& common_parameters);
  server_options(const common_options::parameters& common_parameters,
                 const parameters& parameters);

  parameters server_parameters;
};

inline client_options::client_options(const common_options::parameters& common_parameters)
    : client_options(common_parameters,
                     parameters{}) {
}

inline client_options::client_options(const common_options::parameters& common_parameters,
                                      const parameters& parameters)
    : common_options(common_parameters),
      client_parameters(parameters) {
}

inline server_options::server_options(const common_options::parameters& common_parameters)
    : server_options(common_parameters,
                     parameters{}) {
}

inline server_options::server_options(const common_options::parameters& common_parameters,
                                      const parameters& parameters)
    : common_options(common_parameters),
      server_parameters(parameters) {
}

} // namespace pqrs::unix_domain_stream
