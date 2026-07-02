#pragma once

#include "logger.hpp"
#include "virtual_hid_device_service_clients_manager.hpp"
#include <cstring>
#include <filesystem>
#include <pqrs/dispatcher.hpp>
#include <pqrs/karabiner/driverkit/virtual_hid_device_driver.hpp>
#include <pqrs/karabiner/driverkit/virtual_hid_device_service.hpp>
#include <pqrs/unix_domain_stream.hpp>
#include <type_traits>
#include <vector>

class virtual_hid_device_service_server final : public pqrs::dispatcher::extra::dispatcher_client {
public:
  virtual_hid_device_service_server(pqrs::not_null_shared_ptr_t<pqrs::cf::run_loop_thread> run_loop_thread)
      : dispatcher_client(),
        run_loop_thread_(run_loop_thread),
        create_server_retry_timer_(*this) {
    //
    // Preparation
    //

    virtual_hid_device_service_clients_manager_ = std::make_unique<virtual_hid_device_service_clients_manager>(weak_dispatcher_,
                                                                                                               run_loop_thread_);
    virtual_hid_device_service_clients_manager_->status_changed.connect([this](auto peer_id, const auto& response) {
      async_deliver_status(peer_id,
                           response);
    });

    //
    // Creation
    //

    create_server();

    logger::get_logger()->debug("virtual_hid_device_service_server is initialized");
  }

  ~virtual_hid_device_service_server() override {
    detach_from_dispatcher([this] {
      create_server_retry_timer_.stop();
      server_ = nullptr;

      virtual_hid_device_service_clients_manager_ = nullptr;
    });

    logger::get_logger()->debug("virtual_hid_device_service_server is terminated");
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

  bool create_rootonly_directory() const {
    std::error_code error_code;
    std::filesystem::create_directories(
        pqrs::karabiner::driverkit::virtual_hid_device_service::constants::get_rootonly_directory(),
        error_code);
    if (error_code) {
      logger::get_logger()->error(
          "virtual_hid_device_service_server::{0} create_directories error: {1}",
          __func__,
          error_code.message());
      return false;
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
      return false;
    }

    return true;
  }

  void create_server() {
    create_server_retry_timer_.stop();

    // Prepare socket directories.
    if (!prepare_socket_directories()) {
      start_create_server_retry_timer();
      return;
    }

    auto options = pqrs::unix_domain_stream::server_options(
        {
            .max_message_size = pqrs::karabiner::driverkit::virtual_hid_device_service::constants::unix_domain_stream_max_message_size,
        },
        {
            .bind_retry_interval = std::chrono::milliseconds(1000),
            .socket_path_health_check_interval = std::chrono::milliseconds(3000),
        });

    server_ = std::make_unique<pqrs::unix_domain_stream::server>(
        weak_dispatcher_,
        pqrs::karabiner::driverkit::virtual_hid_device_service::constants::get_server_socket_file_path(),
        options);

    server_->bound.connect([] {
      logger::get_logger()->debug("virtual_hid_device_service_server: bound");
    });

    server_->bind_failed.connect([this](auto&& error_code) {
      logger::get_logger()->error("virtual_hid_device_service_server: bind_failed: {0}",
                                  error_code.message());

      // If the socket directory is deleted for any reason,
      // bind_failed will be triggered, so recreate the directory each time.
      if (!prepare_socket_directories()) {
        if (server_) {
          server_->async_stop();
        }

        enqueue_to_dispatcher([this] {
          server_ = nullptr;

          start_create_server_retry_timer();
        });
      }
    });

    server_->closed.connect([] {
      logger::get_logger()->debug("virtual_hid_device_service_server: closed");
    });

    server_->peer_connected.connect([this](auto peer_id, auto&&) {
      logger::get_logger()->debug("virtual_hid_device_service_server: peer_connected ({0})",
                                  peer_id);

      virtual_hid_device_service_clients_manager_->create_client(peer_id);
      virtual_hid_device_service_clients_manager_->async_check_status_changed(peer_id);
    });

    server_->peer_closed.connect([this](auto peer_id) {
      logger::get_logger()->debug("virtual_hid_device_service_server: peer_closed ({0})",
                                  peer_id);

      virtual_hid_device_service_clients_manager_->erase_client(peer_id);
    });

    server_->peer_error_occurred.connect([](auto peer_id, auto&& error_code) {
      logger::get_logger()->debug("virtual_hid_device_service_server: peer_error_occurred ({0}): {1}",
                                  peer_id,
                                  error_code.message());
    });

    server_->request_received.connect([this](auto peer_id, auto request_id, auto&& buffer) {
      auto respond_empty = [this, peer_id, request_id] {
        if (server_) {
          server_->async_respond(peer_id, request_id, {});
        }
      };

      if (buffer->empty()) {
        logger::get_logger()->error("virtual_hid_device_service_server: payload is empty");
        respond_empty();
        return;
      }

      size_t offset = 0;

      //
      // Read common data
      //
      // buffer[0]: client_protocol_version[0]
      // buffer[1]: client_protocol_version[1]
      // buffer[2]: pqrs::karabiner::driverkit::virtual_hid_device_service::request

      pqrs::karabiner::driverkit::client_protocol_version::value_t received_client_protocol_version(0);
      if (!read_data(*buffer, offset, received_client_protocol_version)) {
        logger::get_logger()->error("virtual_hid_device_service_server: payload is not enough");
        respond_empty();
        return;
      }

      pqrs::karabiner::driverkit::virtual_hid_device_service::request request{};
      if (!read_data(*buffer, offset, request)) {
        logger::get_logger()->error("virtual_hid_device_service_server: payload is not enough");
        respond_empty();
        return;
      }

      //
      // Check client protocol version
      //

      if (received_client_protocol_version != pqrs::karabiner::driverkit::client_protocol_version::embedded_client_protocol_version) {
        logger::get_logger()->warn("client protocol version is mismatched: expected: {0}, actual: {1}",
                                   type_safe::get(pqrs::karabiner::driverkit::client_protocol_version::embedded_client_protocol_version),
                                   type_safe::get(received_client_protocol_version));
        respond_empty();
        return;
      }

      //
      // Handle request
      //

      switch (request) {
        case pqrs::karabiner::driverkit::virtual_hid_device_service::request::virtual_hid_keyboard_initialize: {
          logger::get_logger()->debug("peer_id:{0} received request::virtual_hid_keyboard_initialize",
                                      peer_id);

          auto payload_size = buffer->size() - offset;
          if (sizeof(pqrs::karabiner::driverkit::virtual_hid_device_service::virtual_hid_keyboard_parameters) != payload_size) {
            logger::get_logger()->warn("virtual_hid_device_service_server: received: virtual_hid_keyboard_initialize buffer size error");
            respond_empty();
            return;
          }

          pqrs::karabiner::driverkit::virtual_hid_device_service::virtual_hid_keyboard_parameters parameters;
          std::memcpy(std::addressof(parameters),
                      buffer->data() + offset,
                      sizeof(parameters));

          virtual_hid_device_service_clients_manager_->initialize_keyboard(peer_id,
                                                                           parameters);
          break;
        }

        case pqrs::karabiner::driverkit::virtual_hid_device_service::request::virtual_hid_keyboard_terminate:
          logger::get_logger()->debug("peer_id:{0} received request::virtual_hid_keyboard_terminate",
                                      peer_id);

          virtual_hid_device_service_clients_manager_->terminate_keyboard(peer_id);
          break;

        case pqrs::karabiner::driverkit::virtual_hid_device_service::request::virtual_hid_keyboard_reset:
          virtual_hid_device_service_clients_manager_->virtual_hid_keyboard_reset(peer_id);
          break;

        case pqrs::karabiner::driverkit::virtual_hid_device_service::request::virtual_hid_pointing_initialize: {
          logger::get_logger()->debug("peer_id:{0} received request::virtual_hid_pointing_initialize",
                                      peer_id);

          virtual_hid_device_service_clients_manager_->initialize_pointing(peer_id);
          break;
        }

        case pqrs::karabiner::driverkit::virtual_hid_device_service::request::virtual_hid_pointing_terminate:
          logger::get_logger()->debug("peer_id:{0} received request::virtual_hid_pointing_terminate",
                                      peer_id);

          virtual_hid_device_service_clients_manager_->terminate_pointing(peer_id);
          break;

        case pqrs::karabiner::driverkit::virtual_hid_device_service::request::virtual_hid_pointing_reset:
          virtual_hid_device_service_clients_manager_->virtual_hid_pointing_reset(peer_id);
          break;

        case pqrs::karabiner::driverkit::virtual_hid_device_service::request::post_keyboard_input_report:
          virtual_hid_device_service_clients_manager_->post_keyboard_report(
              peer_id,
              buffer,
              offset,
              pqrs::karabiner::driverkit::virtual_hid_device_driver::user_client_method::virtual_hid_keyboard_post_report,
              "virtual_hid_keyboard_post_report(keyboard_input)",
              sizeof(pqrs::karabiner::driverkit::virtual_hid_device_driver::hid_report::keyboard_input));
          respond_empty();
          return;

        case pqrs::karabiner::driverkit::virtual_hid_device_service::request::post_consumer_input_report:
          virtual_hid_device_service_clients_manager_->post_keyboard_report(
              peer_id,
              buffer,
              offset,
              pqrs::karabiner::driverkit::virtual_hid_device_driver::user_client_method::virtual_hid_keyboard_post_report,
              "virtual_hid_keyboard_post_report(consumer_input)",
              sizeof(pqrs::karabiner::driverkit::virtual_hid_device_driver::hid_report::consumer_input));
          respond_empty();
          return;

        case pqrs::karabiner::driverkit::virtual_hid_device_service::request::post_apple_vendor_keyboard_input_report:
          virtual_hid_device_service_clients_manager_->post_keyboard_report(
              peer_id,
              buffer,
              offset,
              pqrs::karabiner::driverkit::virtual_hid_device_driver::user_client_method::virtual_hid_keyboard_post_report,
              "virtual_hid_keyboard_post_report(apple_vendor_keyboard_input)",
              sizeof(pqrs::karabiner::driverkit::virtual_hid_device_driver::hid_report::apple_vendor_keyboard_input));
          respond_empty();
          return;

        case pqrs::karabiner::driverkit::virtual_hid_device_service::request::post_apple_vendor_top_case_input_report:
          virtual_hid_device_service_clients_manager_->post_keyboard_report(
              peer_id,
              buffer,
              offset,
              pqrs::karabiner::driverkit::virtual_hid_device_driver::user_client_method::virtual_hid_keyboard_post_report,
              "virtual_hid_keyboard_post_report(apple_vendor_top_case_input)",
              sizeof(pqrs::karabiner::driverkit::virtual_hid_device_driver::hid_report::apple_vendor_top_case_input));
          respond_empty();
          return;

        case pqrs::karabiner::driverkit::virtual_hid_device_service::request::post_generic_desktop_input_report:
          virtual_hid_device_service_clients_manager_->post_keyboard_report(
              peer_id,
              buffer,
              offset,
              pqrs::karabiner::driverkit::virtual_hid_device_driver::user_client_method::virtual_hid_keyboard_post_report,
              "virtual_hid_keyboard_post_report(generic_desktop_input)",
              sizeof(pqrs::karabiner::driverkit::virtual_hid_device_driver::hid_report::generic_desktop_input));
          respond_empty();
          return;

        case pqrs::karabiner::driverkit::virtual_hid_device_service::request::post_pointing_input_report:
          virtual_hid_device_service_clients_manager_->post_pointing_report(
              peer_id,
              buffer,
              offset,
              pqrs::karabiner::driverkit::virtual_hid_device_driver::user_client_method::virtual_hid_pointing_post_report,
              "virtual_hid_pointing_post_report(pointing_input)",
              sizeof(pqrs::karabiner::driverkit::virtual_hid_device_driver::hid_report::pointing_input));
          respond_empty();
          return;

        default:
          logger::get_logger()->warn("virtual_hid_device_service_server: unknown request");
          respond_empty();
          return;
      }

      if (server_) {
        server_->async_respond(peer_id,
                               request_id,
                               virtual_hid_device_service_clients_manager_->make_response(peer_id));
      }
    });

    server_->async_start();
  }

  bool prepare_socket_directories() const {
    return create_rootonly_directory();
  }

  void start_create_server_retry_timer() {
    logger::get_logger()->debug("virtual_hid_device_service_server: retry create_server");

    create_server_retry_timer_.start(
        [this] {
          create_server();
        },
        std::chrono::milliseconds(1000));
  }

  void async_deliver_status(pqrs::unix_domain_stream::peer_id peer_id,
                            const std::vector<uint8_t>& response) {
    if (!server_) {
      return;
    }

    server_->async_request(
        peer_id,
        response,
        [peer_id](auto&& error_code, auto&&) {
          if (error_code) {
            logger::get_logger()->debug("virtual_hid_device_service_server: status delivery failed ({0}): {1}",
                                        peer_id,
                                        error_code.message());
          }
        });
  }

  pqrs::not_null_shared_ptr_t<pqrs::cf::run_loop_thread> run_loop_thread_;

  pqrs::dispatcher::extra::timer create_server_retry_timer_;
  std::unique_ptr<virtual_hid_device_service_clients_manager> virtual_hid_device_service_clients_manager_;
  std::unique_ptr<pqrs::unix_domain_stream::server> server_;
};
