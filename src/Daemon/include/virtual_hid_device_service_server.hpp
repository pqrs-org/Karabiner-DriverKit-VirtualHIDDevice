#pragma once

#include "logger.hpp"
#include "virtual_hid_device_service_clients_manager.hpp"
#include <cstring>
#include <filesystem>
#include <pqrs/dispatcher.hpp>
#include <pqrs/karabiner/driverkit/virtual_hid_device_driver.hpp>
#include <pqrs/karabiner/driverkit/virtual_hid_device_service.hpp>
#include <pqrs/local_datagram.hpp>
#include <sstream>
#include <type_traits>
#include <vector>

class virtual_hid_device_service_server final : public pqrs::dispatcher::extra::dispatcher_client {
public:
  virtual_hid_device_service_server(pqrs::not_null_shared_ptr_t<pqrs::cf::run_loop_thread> run_loop_thread)
      : dispatcher_client(),
        run_loop_thread_(run_loop_thread) {
    //
    // Preparation
    //

    virtual_hid_device_service_clients_manager_ = std::make_unique<virtual_hid_device_service_clients_manager>(run_loop_thread_);

    //
    // Creation
    //

    create_server();

    logger::get_logger()->info("virtual_hid_device_service_server is initialized");
  }

  virtual ~virtual_hid_device_service_server() {
    detach_from_dispatcher([this] {
      server_ = nullptr;

      virtual_hid_device_service_clients_manager_ = nullptr;
    });

    logger::get_logger()->info("virtual_hid_device_service_server is terminated");
  }

private:
  template <typename T>
  static bool read_data(const std::vector<uint8_t>& buffer,
                        size_t& offset,
                        T& value) {
    static_assert(std::is_trivially_copyable_v<T>);

    if (offset > buffer.size() ||
        buffer.size() - offset < sizeof(T)) {
      return false;
    }

    std::memcpy(std::addressof(value),
                buffer.data() + offset,
                sizeof(T));
    offset += sizeof(T);
    return true;
  }

  void create_rootonly_directory() const {
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

  std::string server_socket_file_path() const {
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

  void create_server() {
    // Remove old files and prepare a socket directores.
    prepare_socket_directories();

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

    server_->bind_failed.connect([this](auto&& error_code) {
      logger::get_logger()->error("virtual_hid_device_service_server: bind_failed: {0}",
                                  error_code.message());

      // If the socket directory is deleted for any reason,
      // bind_failed will be triggered, so recreate the directory each time.
      prepare_socket_directories();
    });

    server_->closed.connect([] {
      logger::get_logger()->info("virtual_hid_device_service_server: closed");
    });

    server_->received.connect([this](auto&& buffer, auto&& sender_endpoint) {
      if (!pqrs::local_datagram::non_empty_filesystem_endpoint_path(*sender_endpoint)) {
        logger::get_logger()->error("virtual_hid_device_service_server: sender_endpoint path is empty");
        return;
      }

      if (buffer->empty()) {
        logger::get_logger()->error("virtual_hid_device_service_server: payload is empty");
        return;
      }

      auto sender_endpoint_filename = std::filesystem::path(sender_endpoint->path()).filename();

      size_t offset = 0;

      //
      // Read common data
      //
      // buffer[0]: 'c'
      // buffer[1]: 'p'
      // buffer[2]: client_protocol_version[0]
      // buffer[3]: client_protocol_version[1]
      // buffer[4]: pqrs::karabiner::driverkit::virtual_hid_device_service::request

      if (buffer->size() < 2 ||
          (*buffer)[0] != 'c' ||
          (*buffer)[1] != 'p') {
        logger::get_logger()->error("virtual_hid_device_service_server: unknown request");
        return;
      }
      offset = 2;

      pqrs::karabiner::driverkit::client_protocol_version::value_t received_client_protocol_version(0);
      if (!read_data(*buffer, offset, received_client_protocol_version)) {
        logger::get_logger()->error("virtual_hid_device_service_server: payload is not enough");
        return;
      }

      pqrs::karabiner::driverkit::virtual_hid_device_service::request request{};
      if (!read_data(*buffer, offset, request)) {
        logger::get_logger()->error("virtual_hid_device_service_server: payload is not enough");
        return;
      }

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

          auto payload_size = buffer->size() - offset;
          if (sizeof(pqrs::karabiner::driverkit::virtual_hid_device_service::virtual_hid_keyboard_parameters) != payload_size) {
            logger::get_logger()->warn("virtual_hid_device_service_server: received: virtual_hid_keyboard_initialize buffer size error");
            return;
          }

          pqrs::karabiner::driverkit::virtual_hid_device_service::virtual_hid_keyboard_parameters parameters;
          std::memcpy(std::addressof(parameters),
                      buffer->data() + offset,
                      sizeof(parameters));

          virtual_hid_device_service_clients_manager_->create_client(sender_endpoint->path());
          virtual_hid_device_service_clients_manager_->initialize_keyboard(sender_endpoint->path(),
                                                                           parameters);
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
          virtual_hid_device_service_clients_manager_->post_keyboard_report(
              sender_endpoint->path(),
              buffer,
              offset,
              pqrs::karabiner::driverkit::virtual_hid_device_driver::user_client_method::virtual_hid_keyboard_post_report,
              "virtual_hid_keyboard_post_report(keyboard_input)",
              sizeof(pqrs::karabiner::driverkit::virtual_hid_device_driver::hid_report::keyboard_input));
          break;

        case pqrs::karabiner::driverkit::virtual_hid_device_service::request::post_consumer_input_report:
          virtual_hid_device_service_clients_manager_->post_keyboard_report(
              sender_endpoint->path(),
              buffer,
              offset,
              pqrs::karabiner::driverkit::virtual_hid_device_driver::user_client_method::virtual_hid_keyboard_post_report,
              "virtual_hid_keyboard_post_report(consumer_input)",
              sizeof(pqrs::karabiner::driverkit::virtual_hid_device_driver::hid_report::consumer_input));
          break;

        case pqrs::karabiner::driverkit::virtual_hid_device_service::request::post_apple_vendor_keyboard_input_report:
          virtual_hid_device_service_clients_manager_->post_keyboard_report(
              sender_endpoint->path(),
              buffer,
              offset,
              pqrs::karabiner::driverkit::virtual_hid_device_driver::user_client_method::virtual_hid_keyboard_post_report,
              "virtual_hid_keyboard_post_report(apple_vendor_keyboard_input)",
              sizeof(pqrs::karabiner::driverkit::virtual_hid_device_driver::hid_report::apple_vendor_keyboard_input));
          break;

        case pqrs::karabiner::driverkit::virtual_hid_device_service::request::post_apple_vendor_top_case_input_report:
          virtual_hid_device_service_clients_manager_->post_keyboard_report(
              sender_endpoint->path(),
              buffer,
              offset,
              pqrs::karabiner::driverkit::virtual_hid_device_driver::user_client_method::virtual_hid_keyboard_post_report,
              "virtual_hid_keyboard_post_report(apple_vendor_top_case_input)",
              sizeof(pqrs::karabiner::driverkit::virtual_hid_device_driver::hid_report::apple_vendor_top_case_input));
          break;

        case pqrs::karabiner::driverkit::virtual_hid_device_service::request::post_generic_desktop_input_report:
          virtual_hid_device_service_clients_manager_->post_keyboard_report(
              sender_endpoint->path(),
              buffer,
              offset,
              pqrs::karabiner::driverkit::virtual_hid_device_driver::user_client_method::virtual_hid_keyboard_post_report,
              "virtual_hid_keyboard_post_report(generic_desktop_input)",
              sizeof(pqrs::karabiner::driverkit::virtual_hid_device_driver::hid_report::generic_desktop_input));
          break;

        case pqrs::karabiner::driverkit::virtual_hid_device_service::request::post_pointing_input_report:
          virtual_hid_device_service_clients_manager_->post_pointing_report(
              sender_endpoint->path(),
              buffer,
              offset,
              pqrs::karabiner::driverkit::virtual_hid_device_driver::user_client_method::virtual_hid_pointing_post_report,
              "virtual_hid_pointing_post_report(pointing_input)",
              sizeof(pqrs::karabiner::driverkit::virtual_hid_device_driver::hid_report::pointing_input));
          break;
      }
    });

    server_->async_start();
  }

  void prepare_socket_directories() const {
    create_rootonly_directory();

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
  }

  pqrs::not_null_shared_ptr_t<pqrs::cf::run_loop_thread> run_loop_thread_;

  std::unique_ptr<virtual_hid_device_service_clients_manager> virtual_hid_device_service_clients_manager_;
  std::unique_ptr<pqrs::local_datagram::server> server_;
};
