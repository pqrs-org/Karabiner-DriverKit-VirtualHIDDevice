#pragma once

// (C) Copyright Takayama Fumihiko 2026.
// Distributed under the Boost Software License, Version 1.0.
// (See https://www.boost.org/LICENSE_1_0.txt)

#include <atomic>
#include <cstdint>
#include <functional>
#include <pqrs/dispatcher.hpp>
#include <utility>

namespace pqrs::unix_domain_stream::impl {

// Groups notifications belonging to one running session. Stop or invalidate
// cancels the whole group, including notifications not yet posted by I/O.
// Unlike debouncing, all notifications in the current group are delivered.
//
// Client/server notification lifetime:
//
// After `async_stop()` is processed on the dispatcher, queued lifecycle and data
// notifications from the previous running session are discarded, including failures
// and close notifications. Restarting does not revive those notifications.
// `client::async_invalidate_connection()` similarly discards notifications from the
// previous connection session while allowing a new connection to be established.
//
// Pending request callbacks still report their completion or cancellation while the
// owner exists. Server peer cleanup remains keyed by peer ID, so a peer closed by
// the server or its remote endpoint is reported at most once. Server stop or
// destruction suppresses outstanding peer notifications. Destruction detaches the
// owner from the dispatcher and cancels its remaining callbacks.
//
// The owner must detach its dispatcher_client before destroying this scope.
class notification_scope final {
public:
  using token = uint64_t;

  explicit notification_scope(dispatcher::extra::dispatcher_client& owner)
      : owner_(owner) {
  }

  // These lifecycle operations run on the dispatcher thread.
  void start() noexcept {
    running_ = true;
  }

  void stop() noexcept {
    running_ = false;
    invalidate();
  }

  void invalidate() noexcept {
    ++generation_;
  }

  // Reads and enqueue may also run on the I/O thread.
  [[nodiscard]] bool running() const noexcept {
    return running_;
  }

  [[nodiscard]] token capture() const noexcept {
    return generation_;
  }

  [[nodiscard]] bool is_current(token value) const noexcept {
    return running_ && generation_ == value;
  }

  bool enqueue(token value, std::function<void()> function) const {
    return owner_.enqueue_to_dispatcher([this, value, function = std::move(function)] {
      if (is_current(value)) {
        function();
      }
    });
  }

private:
  dispatcher::extra::dispatcher_client& owner_;
  std::atomic_bool running_ = false;
  std::atomic<token> generation_ = 0;
};

} // namespace pqrs::unix_domain_stream::impl
