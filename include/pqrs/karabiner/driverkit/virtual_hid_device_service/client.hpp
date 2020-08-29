#pragma once

// (C) Copyright Takayama Fumihiko 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See http://www.boost.org/LICENSE_1_0.txt)

#include "constants.hpp"
#include "request.hpp"
#include "response.hpp"
#include <pqrs/dispatcher.hpp>
#include <pqrs/hid.hpp>
#include <pqrs/local_datagram.hpp>

namespace pqrs {
namespace karabiner {
namespace driverkit {
namespace virtual_hid_device_service {
class client final : public dispatcher::extra::dispatcher_client {
public:
  // Signals (invoked from the dispatcher thread)

  nod::signal<void(void)> connected;
  nod::signal<void(const asio::error_code&)> connect_failed;
  nod::signal<void(void)> closed;
  nod::signal<void(const asio::error_code&)> error_occurred;
  nod::signal<void(bool)> virtual_hid_keyboard_ready_callback;
  nod::signal<void(bool)> virtual_hid_pointing_ready_callback;

  // Methods

  client(const std::string& client_socket_file_path) : dispatcher_client() {
    create_client(client_socket_file_path);
  }

  virtual ~client(void) {
    detach_from_dispatcher([this] {
      client_ = nullptr;
    });
  }

  void async_start(void) {
    if (client_) {
      client_->async_start();
    }
  }

  void async_virtual_hid_keyboard_initialize(hid::country_code::value_t country_code) {
    async_send(request::virtual_hid_keyboard_initialize, country_code);
  }

  void async_virtual_hid_keyboard_terminate(void) {
    async_send(request::virtual_hid_keyboard_terminate);
  }

  void async_virtual_hid_keyboard_ready(void) {
    async_send(request::virtual_hid_keyboard_ready);
  }

  void async_virtual_hid_keyboard_reset(void) {
    async_send(request::virtual_hid_keyboard_reset);
  }

  void async_virtual_hid_pointing_initialize(void) {
    async_send(request::virtual_hid_pointing_initialize);
  }

  void async_virtual_hid_pointing_terminate(void) {
    async_send(request::virtual_hid_pointing_terminate);
  }

  void async_virtual_hid_pointing_ready(void) {
    async_send(request::virtual_hid_pointing_ready);
  }

  void async_virtual_hid_pointing_reset(void) {
    async_send(request::virtual_hid_pointing_reset);
  }

  void async_post_report(const virtual_hid_device_driver::hid_report::keyboard_input& report) {
    async_send(request::post_keyboard_input_report, report);
  }

  void async_post_report(const virtual_hid_device_driver::hid_report::consumer_input& report) {
    async_send(request::post_consumer_input_report, report);
  }

  void async_post_report(const virtual_hid_device_driver::hid_report::apple_vendor_keyboard_input& report) {
    async_send(request::post_apple_vendor_keyboard_input_report, report);
  }

  void async_post_report(const virtual_hid_device_driver::hid_report::apple_vendor_top_case_input& report) {
    async_send(request::post_apple_vendor_top_case_input_report, report);
  }

  void async_post_report(const virtual_hid_device_driver::hid_report::pointing_input& report) {
    async_send(request::post_pointing_input_report, report);
  }

private:
  void create_client(const std::string& client_socket_file_path) {
    client_ = std::make_unique<local_datagram::client>(weak_dispatcher_,
                                                       constants::server_socket_file_path.data(),
                                                       client_socket_file_path,
                                                       constants::local_datagram_buffer_size);
    client_->set_server_check_interval(std::chrono::milliseconds(3000));
    client_->set_reconnect_interval(std::chrono::milliseconds(1000));

    client_->connected.connect([this] {
      enqueue_to_dispatcher([this] {
        connected();
      });
    });

    client_->connect_failed.connect([this](auto&& error_code) {
      enqueue_to_dispatcher([this, error_code] {
        connect_failed(error_code);
      });
    });

    client_->closed.connect([this] {
      enqueue_to_dispatcher([this] {
        closed();
      });
    });

    client_->closed.connect([this] {
      enqueue_to_dispatcher([this] {
        closed();
      });
    });

    client_->error_occurred.connect([this](auto&& error_code) {
      enqueue_to_dispatcher([this, error_code] {
        error_occurred(error_code);
      });
    });

    client_->received.connect([this](auto&& buffer, auto&& sender_endpoint) {
      if (buffer) {
        if (buffer->empty()) {
          return;
        }

        auto p = &((*buffer)[0]);
        auto size = buffer->size();

        auto r = response(*p);
        ++p;
        --size;

        switch (r) {
          case response::none:
            break;

          case response::virtual_hid_keyboard_ready_result:
            if (size == 1) {
              virtual_hid_keyboard_ready_callback(*p);
            }
            break;

          case response::virtual_hid_pointing_ready_result:
            if (size == 1) {
              virtual_hid_pointing_ready_callback(*p);
            }
            break;
        }
      }
    });
  }

  void async_send(request r) {
    enqueue_to_dispatcher([this, r] {
      if (client_) {
        uint8_t buffer[] = {
            static_cast<std::underlying_type<request>::type>(r),
        };
        client_->async_send(buffer, sizeof(buffer));
      }
    });
  }

  template <typename T>
  void async_send(request r, const T& data) {
    enqueue_to_dispatcher([this, r, data] {
      if (client_) {
        std::vector<uint8_t> buffer;
        append_data(buffer, r);
        append_data(buffer, data);
        client_->async_send(buffer);
      }
    });
  }

  template <typename T>
  void append_data(std::vector<uint8_t>& buffer, const T& data) const {
    auto size = buffer.size();
    buffer.resize(size + sizeof(data));
    *(reinterpret_cast<T*>(&(buffer[size]))) = data;
  }

  std::unique_ptr<local_datagram::client> client_;
};
} // namespace virtual_hid_device_service
} // namespace driverkit
} // namespace karabiner
} // namespace pqrs
