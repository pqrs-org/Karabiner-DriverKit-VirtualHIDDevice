#pragma once

// (C) Copyright Takayama Fumihiko 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See https://www.boost.org/LICENSE_1_0.txt)

#include "constants.hpp"
#include "parameters.hpp"
#include "request.hpp"
#include "response.hpp"
#include <cstring>
#include <pqrs/dispatcher.hpp>
#include <pqrs/hid.hpp>
#include <pqrs/unix_domain_stream.hpp>
#include <ranges>
#include <type_traits>

namespace pqrs::karabiner::driverkit::virtual_hid_device_service {
class client final : public dispatcher::extra::dispatcher_client {
public:
  // Signals (invoked from the dispatcher thread)

  nod::signal<void(const std::string&)> warning_reported;
  nod::signal<void()> connected;
  nod::signal<void(const asio::error_code&)> connect_failed;
  nod::signal<void()> closed;
  nod::signal<void(const asio::error_code&)> error_occurred;
  nod::signal<void(bool)> driver_activated;
  nod::signal<void(bool)> driver_connected;
  nod::signal<void(bool)> driver_version_mismatched;
  nod::signal<void(bool)> virtual_hid_keyboard_ready;
  nod::signal<void(bool)> virtual_hid_pointing_ready;

  // Methods

  client()
      : dispatcher_client(),
        status_request_timer_(*this) {
  }

  ~client() override {
    detach_from_dispatcher([this] {
      status_request_timer_.stop();
      client_ = nullptr;
    });
  }

  void async_start() {
    enqueue_to_dispatcher([this] {
      if (client_) {
        return;
      }

      create_client();

      if (client_) {
        client_->async_start();
      }
    });
  }

  void async_stop() {
    enqueue_to_dispatcher([this] {
      status_request_timer_.stop();
      client_ = nullptr;

      clear_state();
    });
  }

  void async_virtual_hid_keyboard_initialize(const pqrs::karabiner::driverkit::virtual_hid_device_service::virtual_hid_keyboard_parameters& parameters,
                                             bool force = false) {
    if (!force) {
      if (last_virtual_hid_keyboard_ready_ == true &&
          last_virtual_hid_keyboard_parameters_ == parameters) {
        return;
      }
    }

    last_virtual_hid_keyboard_parameters_ = parameters;

    async_request(make_request_buffer(request::virtual_hid_keyboard_initialize,
                                      parameters));
  }

  void async_virtual_hid_keyboard_terminate() {
    async_request(make_request_buffer(request::virtual_hid_keyboard_terminate));
  }

  void async_virtual_hid_keyboard_reset() {
    async_request(make_request_buffer(request::virtual_hid_keyboard_reset));
  }

  void async_virtual_hid_pointing_initialize(bool force = false) {
    if (!force) {
      if (last_virtual_hid_pointing_ready_ == true) {
        return;
      }
    }

    async_request(make_request_buffer(request::virtual_hid_pointing_initialize));
  }

  void async_virtual_hid_pointing_terminate() {
    async_request(make_request_buffer(request::virtual_hid_pointing_terminate));
  }

  void async_virtual_hid_pointing_reset() {
    async_request(make_request_buffer(request::virtual_hid_pointing_reset));
  }

  void async_post_report(const virtual_hid_device_driver::hid_report::keyboard_input& report) {
    async_request(make_request_buffer(request::post_keyboard_input_report,
                                      report));
  }

  void async_post_report(const virtual_hid_device_driver::hid_report::consumer_input& report) {
    async_request(make_request_buffer(request::post_consumer_input_report,
                                      report));
  }

  void async_post_report(const virtual_hid_device_driver::hid_report::apple_vendor_keyboard_input& report) {
    async_request(make_request_buffer(request::post_apple_vendor_keyboard_input_report,
                                      report));
  }

  void async_post_report(const virtual_hid_device_driver::hid_report::apple_vendor_top_case_input& report) {
    async_request(make_request_buffer(request::post_apple_vendor_top_case_input_report,
                                      report));
  }

  void async_post_report(const virtual_hid_device_driver::hid_report::generic_desktop_input& report) {
    async_request(make_request_buffer(request::post_generic_desktop_input_report,
                                      report));
  }

  void async_post_report(const virtual_hid_device_driver::hid_report::pointing_input& report) {
    async_request(make_request_buffer(request::post_pointing_input_report,
                                      report));
  }

private:
  void clear_state() {
    last_virtual_hid_keyboard_ready_ = std::nullopt;
    virtual_hid_keyboard_ready(false);

    last_virtual_hid_pointing_ready_ = std::nullopt;
    virtual_hid_pointing_ready(false);

    last_virtual_hid_keyboard_parameters_ = std::nullopt;
  }

  void create_client() {
    auto options = pqrs::unix_domain_stream::client_options(
        {
            .max_message_size = constants::unix_domain_stream_max_message_size,
        },
        {
            .reconnect_interval = std::chrono::milliseconds(1000),
        });

    client_ = std::make_unique<unix_domain_stream::client>(weak_dispatcher_,
                                                           constants::get_server_socket_file_path(),
                                                           options);

    client_->connected.connect([this](auto&&) {
      enqueue_to_dispatcher([this] {
        connected();

        status_request_timer_.start(
            [this] {
              async_request(make_request_buffer(request::none));
            },
            std::chrono::milliseconds(1000));
      });
    });

    client_->connect_failed.connect([this](auto&& error_code) {
      auto ec = error_code;

      enqueue_to_dispatcher([this, ec] {
        connect_failed(ec);
      });
    });

    client_->closed.connect([this] {
      enqueue_to_dispatcher([this] {
        status_request_timer_.stop();

        closed();

        clear_state();
      });
    });

    client_->error_occurred.connect([this](auto&& error_code) {
      auto ec = error_code;

      enqueue_to_dispatcher([this, ec] {
        error_occurred(ec);
      });
    });

    client_->peer_verification_failed.connect([this](auto&&) {
      enqueue_to_dispatcher([this] {
        warning_reported("peer_verification_failed");
      });
    });
  }

  void handle_response(const std::shared_ptr<std::vector<uint8_t>>& buffer) {
    if (!buffer ||
        buffer->empty()) {
      return;
    }

    const auto& response_buffer = *buffer;

    for (auto i : std::views::iota(size_t{0}, response_buffer.size() / 2)) {
      auto r = response(response_buffer[i * 2]);
      auto value = response_buffer[i * 2 + 1];
      switch (r) {
        case response::none:
          break;

        case response::driver_activated:
          driver_activated(value);
          break;

        case response::driver_connected:
          driver_connected(value);
          break;

        case response::driver_version_mismatched:
          driver_version_mismatched(value);
          break;

        case response::virtual_hid_keyboard_ready:
          last_virtual_hid_keyboard_ready_ = value;
          virtual_hid_keyboard_ready(value);
          break;

        case response::virtual_hid_pointing_ready:
          last_virtual_hid_pointing_ready_ = value;
          virtual_hid_pointing_ready(value);
          break;
      }
    }
  }

  void async_request(std::shared_ptr<std::vector<uint8_t>> request_buffer) {
    enqueue_to_dispatcher([this, request_buffer] {
      if (client_) {
        client_->async_request(
            *request_buffer,
            [this](auto&& error_code, auto&& response_buffer) {
              auto ec = error_code;
              auto buffer = response_buffer;

              enqueue_to_dispatcher([this, ec, buffer] {
                if (ec) {
                  error_occurred(ec);
                  return;
                }

                if (buffer) {
                  handle_response(buffer);
                }
              });
            });
      }
    });
  }

  std::shared_ptr<std::vector<uint8_t>> make_request_buffer(request r) const {
    auto buffer = std::make_shared<std::vector<uint8_t>>();

    append_data(*buffer, client_protocol_version::embedded_client_protocol_version);
    append_data(*buffer, r);

    return buffer;
  }

  template <typename T>
  std::shared_ptr<std::vector<uint8_t>> make_request_buffer(request r, const T& data) const {
    auto buffer = make_request_buffer(r);

    append_data(*buffer, data);

    return buffer;
  }

  template <typename T>
  void append_data(std::vector<uint8_t>& buffer, const T& data) const {
    static_assert(std::is_trivially_copyable_v<T>);

    auto size = buffer.size();
    buffer.resize(size + sizeof(data));
    std::memcpy(buffer.data() + size,
                std::addressof(data),
                sizeof(data));
  }

  dispatcher::extra::timer status_request_timer_;
  std::unique_ptr<unix_domain_stream::client> client_;

  std::optional<bool> last_virtual_hid_keyboard_ready_;
  std::optional<bool> last_virtual_hid_pointing_ready_;

  std::optional<pqrs::karabiner::driverkit::virtual_hid_device_service::virtual_hid_keyboard_parameters> last_virtual_hid_keyboard_parameters_;
};
} // namespace pqrs::karabiner::driverkit::virtual_hid_device_service
