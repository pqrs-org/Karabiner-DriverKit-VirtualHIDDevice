#pragma once

#include "logger.hpp"
#include "virtual_hid_device_service_clients_manager.hpp"
#include <filesystem>
#include <pqrs/dispatcher.hpp>
#include <pqrs/karabiner/driverkit/virtual_hid_device_driver.hpp>
#include <pqrs/karabiner/driverkit/virtual_hid_device_service.hpp>
#include <pqrs/local_datagram.hpp>
#include <sstream>

class virtual_hid_device_service_server final : public pqrs::dispatcher::extra::dispatcher_client {
public:
  virtual_hid_device_service_server(std::shared_ptr<pqrs::cf::run_loop_thread> run_loop_thread)
      : dispatcher_client(),
        run_loop_thread_(run_loop_thread) {
    //
    // Preparation
    //

    create_rootonly_directory();

    virtual_hid_device_service_clients_manager_ = std::make_unique<virtual_hid_device_service_clients_manager>(run_loop_thread_);

    //
    // Creation
    //

    create_server();

    logger::get_logger()->info("virtual_hid_device_service_server is initialized");
  }

  virtual ~virtual_hid_device_service_server(void) {
    detach_from_dispatcher([this] {
      server_ = nullptr;

      virtual_hid_device_service_clients_manager_ = nullptr;
    });

    logger::get_logger()->info("virtual_hid_device_service_server is terminated");
  }

private:
  void create_rootonly_directory(void) const {
    std::error_code error_code;
    std::filesystem::create_directories(
        pqrs::karabiner::driverkit::virtual_hid_device_service::constants::get_rootonly_directory(),
        error_code);
    if (error_code) {
      logger::get_logger()->error(
          "virtual_hid_device_service_server::{0} create_directories error: {1}",
          __func__,
          error_code.message());
      return;
    }

    std::filesystem::permissions(
        pqrs::karabiner::driverkit::virtual_hid_device_service::constants::get_rootonly_directory(),
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

  std::string server_socket_file_path(void) const {
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();

    std::stringstream ss;
    ss << pqrs::karabiner::driverkit::virtual_hid_device_service::constants::get_server_socket_directory_path().string()
       << "/"
       << std::hex
       << std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count()
       << ".sock";

    return ss.str();
  }

  void create_server(void) {
    // Remove old socket files.
    {
      auto directory_path = pqrs::karabiner::driverkit::virtual_hid_device_service::constants::get_server_socket_directory_path();
      std::error_code ec;
      std::filesystem::remove_all(directory_path, ec);
      std::filesystem::create_directory(directory_path, ec);
    }
    {
      auto directory_path = pqrs::karabiner::driverkit::virtual_hid_device_service::constants::get_server_response_socket_directory_path();
      std::error_code ec;
      std::filesystem::remove_all(directory_path, ec);
      std::filesystem::create_directory(directory_path, ec);
    }
    {
      auto directory_path = pqrs::karabiner::driverkit::virtual_hid_device_service::constants::get_client_socket_directory_path();
      std::error_code ec;
      std::filesystem::remove_all(directory_path, ec);
      std::filesystem::create_directory(directory_path, ec);
    }

    server_ = std::make_unique<pqrs::local_datagram::server>(
        weak_dispatcher_,
        server_socket_file_path(),
        pqrs::karabiner::driverkit::virtual_hid_device_service::constants::local_datagram_buffer_size);
    server_->set_server_check_interval(std::chrono::milliseconds(3000));
    server_->set_reconnect_interval(std::chrono::milliseconds(1000));

    server_->warning_reported.connect([](auto&& message) {
      logger::get_logger()->warn("virtual_hid_device_service_server: {0}",
                                 message);
    });

    server_->bound.connect([] {
      logger::get_logger()->info("virtual_hid_device_service_server: bound");
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
        if (!pqrs::local_datagram::non_empty_filesystem_endpoint_path(*sender_endpoint)) {
          logger::get_logger()->error("virtual_hid_device_service_server: sender_endpoint path is empty");
          return;
        }

        auto sender_endpoint_filename = std::filesystem::path(sender_endpoint->path()).filename();

        auto p = &((*buffer)[0]);
        auto size = buffer->size();

        //
        // Read common data
        //
        // buffer[0]: 'c'
        // buffer[1]: 'p'
        // buffer[2]: client_protocol_version[0]
        // buffer[3]: client_protocol_version[1]
        // buffer[4]: pqrs::karabiner::driverkit::virtual_hid_device_service::request

        if (size-- == 0 || *p++ != 'c' ||
            size-- == 0 || *p++ != 'p') {
          logger::get_logger()->error("virtual_hid_device_service_server: unknown request");
        }

        if (size < sizeof(pqrs::karabiner::driverkit::client_protocol_version::value_t)) {
          logger::get_logger()->error("virtual_hid_device_service_server: payload is not enough");
          return;
        }
        auto received_client_protocol_version = *(reinterpret_cast<pqrs::karabiner::driverkit::client_protocol_version::value_t*>(p));
        p += sizeof(pqrs::karabiner::driverkit::client_protocol_version::value_t);
        size -= sizeof(pqrs::karabiner::driverkit::client_protocol_version::value_t);

        if (size < sizeof(pqrs::karabiner::driverkit::virtual_hid_device_service::request)) {
          logger::get_logger()->error("virtual_hid_device_service_server: payload is not enough");
          return;
        }

        auto request = *(reinterpret_cast<pqrs::karabiner::driverkit::virtual_hid_device_service::request*>(p));
        p += sizeof(pqrs::karabiner::driverkit::virtual_hid_device_service::request);
        size -= sizeof(pqrs::karabiner::driverkit::virtual_hid_device_service::request);

        //
        // Check client protocol version
        //

        if (received_client_protocol_version != pqrs::karabiner::driverkit::client_protocol_version::embedded_client_protocol_version) {
          logger::get_logger()->warn("client protocol version is mismatched: expected: {0}, actual: {1}",
                                     type_safe::get(pqrs::karabiner::driverkit::client_protocol_version::embedded_client_protocol_version),
                                     type_safe::get(received_client_protocol_version));
          return;
        }

        //
        // Handle request
        //

        switch (request) {
          case pqrs::karabiner::driverkit::virtual_hid_device_service::request::none:
            break;

          case pqrs::karabiner::driverkit::virtual_hid_device_service::request::virtual_hid_keyboard_initialize: {
            logger::get_logger()->info("{0} received request::virtual_hid_keyboard_initialize",
                                       sender_endpoint_filename.c_str());

            if (sizeof(pqrs::karabiner::driverkit::virtual_hid_device_service::virtual_hid_keyboard_parameters) != size) {
              logger::get_logger()->warn("virtual_hid_device_service_server: received: virtual_hid_keyboard_initialize buffer size error");
              return;
            }

            auto parameters = reinterpret_cast<pqrs::karabiner::driverkit::virtual_hid_device_service::virtual_hid_keyboard_parameters*>(p);

            virtual_hid_device_service_clients_manager_->create_client(sender_endpoint->path());
            virtual_hid_device_service_clients_manager_->initialize_keyboard(sender_endpoint->path(),
                                                                             *parameters);
            break;
          }

          case pqrs::karabiner::driverkit::virtual_hid_device_service::request::virtual_hid_keyboard_terminate:
            logger::get_logger()->info("{0} received request::virtual_hid_keyboard_terminate",
                                       sender_endpoint_filename.c_str());

            virtual_hid_device_service_clients_manager_->terminate_keyboard(sender_endpoint->path());
            break;

          case pqrs::karabiner::driverkit::virtual_hid_device_service::request::virtual_hid_keyboard_reset:
            virtual_hid_device_service_clients_manager_->virtual_hid_keyboard_reset(sender_endpoint->path());
            break;

          case pqrs::karabiner::driverkit::virtual_hid_device_service::request::virtual_hid_pointing_initialize: {
            logger::get_logger()->info("{0} received request::virtual_hid_pointing_initialize",
                                       sender_endpoint_filename.c_str());

            virtual_hid_device_service_clients_manager_->create_client(sender_endpoint->path());
            virtual_hid_device_service_clients_manager_->initialize_pointing(sender_endpoint->path());
            break;
          }

          case pqrs::karabiner::driverkit::virtual_hid_device_service::request::virtual_hid_pointing_terminate:
            logger::get_logger()->info("{0} received request::virtual_hid_pointing_terminate",
                                       sender_endpoint_filename.c_str());

            virtual_hid_device_service_clients_manager_->terminate_pointing(sender_endpoint->path());
            break;

          case pqrs::karabiner::driverkit::virtual_hid_device_service::request::virtual_hid_pointing_reset:
            virtual_hid_device_service_clients_manager_->virtual_hid_pointing_reset(sender_endpoint->path());
            break;

          case pqrs::karabiner::driverkit::virtual_hid_device_service::request::post_keyboard_input_report:
            virtual_hid_device_service_clients_manager_->post_keyboard_report<pqrs::karabiner::driverkit::virtual_hid_device_driver::hid_report::keyboard_input>(
                sender_endpoint->path(),
                p,
                size);
            break;

          case pqrs::karabiner::driverkit::virtual_hid_device_service::request::post_consumer_input_report:
            virtual_hid_device_service_clients_manager_->post_keyboard_report<pqrs::karabiner::driverkit::virtual_hid_device_driver::hid_report::consumer_input>(
                sender_endpoint->path(),
                p,
                size);
            break;

          case pqrs::karabiner::driverkit::virtual_hid_device_service::request::post_apple_vendor_keyboard_input_report:
            virtual_hid_device_service_clients_manager_->post_keyboard_report<pqrs::karabiner::driverkit::virtual_hid_device_driver::hid_report::apple_vendor_keyboard_input>(
                sender_endpoint->path(),
                p,
                size);
            break;

          case pqrs::karabiner::driverkit::virtual_hid_device_service::request::post_apple_vendor_top_case_input_report:
            virtual_hid_device_service_clients_manager_->post_keyboard_report<pqrs::karabiner::driverkit::virtual_hid_device_driver::hid_report::apple_vendor_top_case_input>(
                sender_endpoint->path(),
                p,
                size);
            break;

          case pqrs::karabiner::driverkit::virtual_hid_device_service::request::post_generic_desktop_input_report:
            virtual_hid_device_service_clients_manager_->post_keyboard_report<pqrs::karabiner::driverkit::virtual_hid_device_driver::hid_report::generic_desktop_input>(
                sender_endpoint->path(),
                p,
                size);
            break;

          case pqrs::karabiner::driverkit::virtual_hid_device_service::request::post_pointing_input_report:
            virtual_hid_device_service_clients_manager_->post_pointing_report<pqrs::karabiner::driverkit::virtual_hid_device_driver::hid_report::pointing_input>(
                sender_endpoint->path(),
                p,
                size);
            break;
        }
      }
    });

    server_->async_start();
  }

  std::shared_ptr<pqrs::cf::run_loop_thread> run_loop_thread_;

  std::unique_ptr<virtual_hid_device_service_clients_manager> virtual_hid_device_service_clients_manager_;
  std::unique_ptr<pqrs::local_datagram::server> server_;
};
