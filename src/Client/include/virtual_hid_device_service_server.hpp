#pragma once

#include "logger.hpp"
#include "virtual_hid_device_service_clients_manager.hpp"
#include <filesystem>
#include <pqrs/dispatcher.hpp>
#include <pqrs/karabiner/driverkit/virtual_hid_device_driver.hpp>
#include <pqrs/karabiner/driverkit/virtual_hid_device_service.hpp>
#include <pqrs/local_datagram.hpp>

class virtual_hid_device_service_server final : public pqrs::dispatcher::extra::dispatcher_client {
public:
  virtual_hid_device_service_server(void) : dispatcher_client(),
                                            ready_timer_(*this) {
    //
    // Preparation
    //

    create_rootonly_directory();

    virtual_hid_device_service_keyboard_clients_manager_ = std::make_unique<virtual_hid_device_service_clients_manager>("keyboard");
    virtual_hid_device_service_keyboard_clients_manager_->all_clients_disconnected.connect([this] {
      terminate_virtual_hid_keyboard_io_service_client();
    });

    virtual_hid_device_service_pointing_clients_manager_ = std::make_unique<virtual_hid_device_service_clients_manager>("pointing");
    virtual_hid_device_service_pointing_clients_manager_->all_clients_disconnected.connect([this] {
      terminate_virtual_hid_pointing_io_service_client();
    });

    //
    // Creation
    //

    create_server();
    create_nop_io_service_client();

    ready_timer_.start(
        [this] {
          if (virtual_hid_keyboard_io_service_client_) {
            virtual_hid_keyboard_io_service_client_->async_virtual_hid_keyboard_ready();
          }

          if (virtual_hid_pointing_io_service_client_) {
            virtual_hid_pointing_io_service_client_->async_virtual_hid_pointing_ready();
          }
        },
        std::chrono::milliseconds(1000));

    logger::get_logger()->info("virtual_hid_device_service_server is initialized");
  }

  virtual ~virtual_hid_device_service_server(void) {
    detach_from_dispatcher([this] {
      ready_timer_.stop();

      server_ = nullptr;
      nop_io_service_client_ = nullptr;
      virtual_hid_keyboard_io_service_client_ = nullptr;
      virtual_hid_pointing_io_service_client_ = nullptr;

      virtual_hid_device_service_keyboard_clients_manager_ = nullptr;
      virtual_hid_device_service_pointing_clients_manager_ = nullptr;
    });

    logger::get_logger()->info("virtual_hid_device_service_server is terminated");
  }

private:
  void create_rootonly_directory(void) const {
    std::error_code error_code;
    std::filesystem::create_directories(
        pqrs::karabiner::driverkit::virtual_hid_device_service::constants::rootonly_directory,
        error_code);
    if (error_code) {
      logger::get_logger()->error(
          "virtual_hid_device_service_server::{0} create_directories error: {1}",
          __func__,
          error_code.message());
      return;
    }

    std::filesystem::permissions(
        pqrs::karabiner::driverkit::virtual_hid_device_service::constants::rootonly_directory,
        std::filesystem::perms::owner_all,
        error_code);
    if (error_code) {
      logger::get_logger()->error(
          "virtual_hid_device_service_server::{0} permissions error: {1}",
          __func__,
          error_code.message());
      return;
    }
  }

  void set_server_socket_file_permissions(void) const {
    std::error_code error_code;
    std::filesystem::permissions(
        pqrs::karabiner::driverkit::virtual_hid_device_service::constants::server_socket_file_path,
        std::filesystem::perms::owner_read | std::filesystem::perms::owner_write,
        error_code);
    if (error_code) {
      logger::get_logger()->error(
          "virtual_hid_device_service_server::{0} permissions error: {1}",
          __func__,
          error_code.message());
      return;
    }
  }

  void create_server(void) {
    server_ = std::make_unique<pqrs::local_datagram::server>(
        weak_dispatcher_,
        pqrs::karabiner::driverkit::virtual_hid_device_service::constants::server_socket_file_path.data(),
        pqrs::karabiner::driverkit::virtual_hid_device_service::constants::local_datagram_buffer_size);
    server_->set_server_check_interval(std::chrono::milliseconds(3000));
    server_->set_reconnect_interval(std::chrono::milliseconds(1000));

    server_->bound.connect([this] {
      logger::get_logger()->info("virtual_hid_device_service_server: bound");

      set_server_socket_file_permissions();
    });

    server_->bind_failed.connect([](auto&& error_code) {
      logger::get_logger()->error("virtual_hid_device_service_server: bind_failed: {0}",
                                  error_code.message());
    });

    server_->closed.connect([] {
      logger::get_logger()->info("virtual_hid_device_service_server: closed");
    });

    server_->received.connect([this](auto&& buffer, auto&& sender_endpoint) {
      if (buffer) {
        if (buffer->empty()) {
          return;
        }

        if (sender_endpoint->path().empty()) {
          logger::get_logger()->error("virtual_hid_device_service_server: sender_endpoint path is empty");
          return;
        }

        auto p = &((*buffer)[0]);
        auto size = buffer->size();

        auto request = pqrs::karabiner::driverkit::virtual_hid_device_service::request(*p);
        ++p;
        --size;

        switch (request) {
          case pqrs::karabiner::driverkit::virtual_hid_device_service::request::none:
            break;

          case pqrs::karabiner::driverkit::virtual_hid_device_service::request::driver_loaded:
            async_send_driver_loaded_result(sender_endpoint);
            break;

          case pqrs::karabiner::driverkit::virtual_hid_device_service::request::driver_version_matched:
            async_send_driver_version_matched_result(sender_endpoint);
            break;

          case pqrs::karabiner::driverkit::virtual_hid_device_service::request::virtual_hid_keyboard_initialize: {
            if (sizeof(pqrs::hid::country_code::value_t) != size) {
              logger::get_logger()->warn("virtual_hid_device_service_server: received: virtual_hid_keyboard_initialize buffer size error");
              return;
            }

            auto country_code = *(reinterpret_cast<pqrs::hid::country_code::value_t*>(p));

            if (virtual_hid_keyboard_country_code_ != country_code) {
              terminate_virtual_hid_keyboard_io_service_client();
              virtual_hid_keyboard_country_code_ = country_code;
            }

            if (!virtual_hid_keyboard_io_service_client_) {
              create_virtual_hid_keyboard_io_service_client(country_code);
            }

            virtual_hid_device_service_keyboard_clients_manager_->async_insert_client(sender_endpoint->path());
            break;
          }

          case pqrs::karabiner::driverkit::virtual_hid_device_service::request::virtual_hid_keyboard_terminate:
            terminate_virtual_hid_keyboard_io_service_client();
            break;

          case pqrs::karabiner::driverkit::virtual_hid_device_service::request::virtual_hid_keyboard_ready:
            async_send_ready_result(
                pqrs::karabiner::driverkit::virtual_hid_device_service::response::virtual_hid_keyboard_ready_result,
                virtual_hid_keyboard_io_service_client_ ? virtual_hid_keyboard_io_service_client_->get_virtual_hid_keyboard_ready() : false,
                sender_endpoint);
            break;

          case pqrs::karabiner::driverkit::virtual_hid_device_service::request::virtual_hid_keyboard_reset:
            if (virtual_hid_keyboard_io_service_client_) {
              virtual_hid_keyboard_io_service_client_->async_virtual_hid_keyboard_reset();
            }
            break;

          case pqrs::karabiner::driverkit::virtual_hid_device_service::request::virtual_hid_pointing_initialize:
            if (!virtual_hid_pointing_io_service_client_) {
              create_virtual_hid_pointing_io_service_client();
            }

            virtual_hid_device_service_pointing_clients_manager_->async_insert_client(sender_endpoint->path());
            break;

          case pqrs::karabiner::driverkit::virtual_hid_device_service::request::virtual_hid_pointing_terminate:
            terminate_virtual_hid_pointing_io_service_client();
            break;

          case pqrs::karabiner::driverkit::virtual_hid_device_service::request::virtual_hid_pointing_ready:
            async_send_ready_result(
                pqrs::karabiner::driverkit::virtual_hid_device_service::response::virtual_hid_pointing_ready_result,
                virtual_hid_pointing_io_service_client_ ? virtual_hid_pointing_io_service_client_->get_virtual_hid_pointing_ready() : false,
                sender_endpoint);
            break;

          case pqrs::karabiner::driverkit::virtual_hid_device_service::request::virtual_hid_pointing_reset:
            if (virtual_hid_pointing_io_service_client_) {
              virtual_hid_pointing_io_service_client_->async_virtual_hid_pointing_reset();
            }
            break;

          case pqrs::karabiner::driverkit::virtual_hid_device_service::request::post_keyboard_input_report:
            async_post_report<pqrs::karabiner::driverkit::virtual_hid_device_driver::hid_report::keyboard_input>(
                virtual_hid_keyboard_io_service_client_,
                p,
                size);
            break;

          case pqrs::karabiner::driverkit::virtual_hid_device_service::request::post_consumer_input_report:
            async_post_report<pqrs::karabiner::driverkit::virtual_hid_device_driver::hid_report::consumer_input>(
                virtual_hid_keyboard_io_service_client_,
                p,
                size);
            break;

          case pqrs::karabiner::driverkit::virtual_hid_device_service::request::post_apple_vendor_keyboard_input_report:
            async_post_report<pqrs::karabiner::driverkit::virtual_hid_device_driver::hid_report::apple_vendor_keyboard_input>(
                virtual_hid_keyboard_io_service_client_,
                p,
                size);
            break;

          case pqrs::karabiner::driverkit::virtual_hid_device_service::request::post_apple_vendor_top_case_input_report:
            async_post_report<pqrs::karabiner::driverkit::virtual_hid_device_driver::hid_report::apple_vendor_top_case_input>(
                virtual_hid_keyboard_io_service_client_,
                p,
                size);
            break;

          case pqrs::karabiner::driverkit::virtual_hid_device_service::request::post_pointing_input_report:
            async_post_report<pqrs::karabiner::driverkit::virtual_hid_device_driver::hid_report::pointing_input>(
                virtual_hid_pointing_io_service_client_,
                p,
                size);
            break;
        }
      }
    });

    server_->async_start();
  }

  // This method is only called in the constructor.
  void create_nop_io_service_client(void) {
    nop_io_service_client_ = std::make_unique<io_service_client>();

    nop_io_service_client_->async_start();
  }

  //
  // virtual_hid_keyboard
  //

  // This method is executed in the dispatcher thread.
  void create_virtual_hid_keyboard_io_service_client(pqrs::hid::country_code::value_t country_code) {
    logger::get_logger()->info("create_virtual_hid_keyboard_io_service_client");

    virtual_hid_keyboard_io_service_client_ = std::make_unique<io_service_client>();

    virtual_hid_keyboard_io_service_client_->opened.connect([this, country_code] {
      virtual_hid_keyboard_io_service_client_->async_virtual_hid_keyboard_initialize(country_code);
    });

    virtual_hid_keyboard_io_service_client_->async_start();
  }

  // This method is executed in the dispatcher thread.
  void terminate_virtual_hid_keyboard_io_service_client(void) {
    logger::get_logger()->info("terminate_virtual_hid_keyboard_io_service_client");

    virtual_hid_keyboard_io_service_client_ = nullptr;
    virtual_hid_keyboard_country_code_ = std::nullopt;
  }

  //
  // virtual_hid_pointing
  //

  // This method is executed in the dispatcher thread.
  void create_virtual_hid_pointing_io_service_client(void) {
    logger::get_logger()->info("create_virtual_hid_pointing_io_service_client");

    virtual_hid_pointing_io_service_client_ = std::make_unique<io_service_client>();

    virtual_hid_pointing_io_service_client_->opened.connect([this] {
      virtual_hid_pointing_io_service_client_->async_virtual_hid_pointing_initialize();
    });

    virtual_hid_pointing_io_service_client_->async_start();
  }

  void terminate_virtual_hid_pointing_io_service_client(void) {
    logger::get_logger()->info("terminate_virtual_hid_pointing_io_service_client");

    virtual_hid_pointing_io_service_client_ = nullptr;
  }

  //
  // async_send to virtual_hid_device_service::client
  //

  // This method is executed in the dispatcher thread.
  void async_send_driver_loaded_result(std::shared_ptr<asio::local::datagram_protocol::endpoint> endpoint) {
    if (server_) {
      if (!endpoint->path().empty()) {
        bool driver_loaded = false;
        if (nop_io_service_client_) {
          driver_loaded = nop_io_service_client_->driver_loaded();
        }

        auto response = pqrs::karabiner::driverkit::virtual_hid_device_service::response::driver_loaded_result;
        uint8_t buffer[] = {
            static_cast<std::underlying_type<decltype(response)>::type>(response),
            driver_loaded,
        };

        server_->async_send(buffer, sizeof(buffer), endpoint);
      }
    }
  }

  // This method is executed in the dispatcher thread.
  void async_send_driver_version_matched_result(std::shared_ptr<asio::local::datagram_protocol::endpoint> endpoint) {
    if (server_) {
      if (!endpoint->path().empty()) {
        bool driver_version_matched = false;
        if (nop_io_service_client_) {
          driver_version_matched = nop_io_service_client_->driver_version_matched();
        }

        auto response = pqrs::karabiner::driverkit::virtual_hid_device_service::response::driver_version_matched_result;
        uint8_t buffer[] = {
            static_cast<std::underlying_type<decltype(response)>::type>(response),
            driver_version_matched,
        };

        server_->async_send(buffer, sizeof(buffer), endpoint);
      }
    }
  }

  // This method is executed in the dispatcher thread.
  void async_send_ready_result(pqrs::karabiner::driverkit::virtual_hid_device_service::response response,
                               std::optional<bool> ready,
                               std::shared_ptr<asio::local::datagram_protocol::endpoint> endpoint) {
    if (server_) {
      if (!endpoint->path().empty()) {
        uint8_t buffer[] = {
            static_cast<std::underlying_type<decltype(response)>::type>(response),
            ready ? *ready : false,
        };

        server_->async_send(buffer, sizeof(buffer), endpoint);
      }
    }
  }

  //
  // async_post to io_service_client
  //

  // This method is executed in the dispatcher thread.
  template <typename T>
  void async_post_report(const std::unique_ptr<io_service_client>& io_service_client,
                         const uint8_t* buffer,
                         size_t buffer_size) {
    if (io_service_client) {
      if (sizeof(T) != buffer_size) {
        logger::get_logger()->warn("virtual_hid_device_service_server: post_report buffer size error");
        return;
      }

      io_service_client->async_post_report(*(reinterpret_cast<const T*>(buffer)));
    }
  }

  // `nop_io_service_client_` does not control virtual devices.
  // It is used for `driver_loaded` and `driver_version_matched`.
  std::unique_ptr<io_service_client> nop_io_service_client_;
  std::unique_ptr<io_service_client> virtual_hid_keyboard_io_service_client_;
  std::unique_ptr<virtual_hid_device_service_clients_manager> virtual_hid_device_service_keyboard_clients_manager_;
  std::optional<pqrs::hid::country_code::value_t> virtual_hid_keyboard_country_code_;
  std::unique_ptr<io_service_client> virtual_hid_pointing_io_service_client_;
  std::unique_ptr<virtual_hid_device_service_clients_manager> virtual_hid_device_service_pointing_clients_manager_;
  std::unique_ptr<pqrs::local_datagram::server> server_;
  pqrs::dispatcher::extra::timer ready_timer_;
};
