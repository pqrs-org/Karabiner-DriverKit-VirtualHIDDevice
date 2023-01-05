#pragma once

#include "logger.hpp"
#include <filesystem>
#include <nod/nod.hpp>
#include <pqrs/dispatcher.hpp>
#include <pqrs/karabiner/driverkit/virtual_hid_device_service.hpp>
#include <pqrs/local_datagram.hpp>
#include <unordered_map>

class virtual_hid_device_service_clients_manager final : public pqrs::dispatcher::extra::dispatcher_client {
public:
  virtual_hid_device_service_clients_manager(std::shared_ptr<pqrs::cf::run_loop_thread> run_loop_thread)
      : dispatcher_client(),
        run_loop_thread_(run_loop_thread) {
  }

  virtual ~virtual_hid_device_service_clients_manager(void) {
    detach_from_dispatcher([this] {
      entries_.clear();
    });
  }

  // This method needs to be called in the dispatcher thread.
  void create_client(const std::string& endpoint_path,
                     pqrs::karabiner::driverkit::driver_version::value_t expected_driver_version) {
    if (!dispatcher_thread()) {
      throw std::logic_error(fmt::format("{0} is called in wrong thread", __func__));
    }

    auto endpoint_filename = std::filesystem::path(endpoint_path).filename();

    logger::get_logger()->info(
        "create a client for virtual_hid_device_service::client: {0}",
        endpoint_filename.c_str());

    {
      auto it = entries_.find(endpoint_path);
      if (it != std::end(entries_)) {
        logger::get_logger()->info(
            "client already exists: {0}",
            endpoint_filename.c_str());
        return;
      }
    }

    //
    // Create pqrs::local_datagram::client
    //

    auto c = std::make_shared<pqrs::local_datagram::client>(
        weak_dispatcher_,
        endpoint_path,
        server_response_socket_file_path(),
        pqrs::karabiner::driverkit::virtual_hid_device_service::constants::local_datagram_buffer_size);

    c->set_server_check_interval(std::chrono::milliseconds(1000));
    c->set_next_heartbeat_deadline(std::chrono::milliseconds(5000));

    c->warning_reported.connect([endpoint_filename](auto&& message) {
      logger::get_logger()->warn(
          "client: {0} {1}",
          endpoint_filename.c_str(),
          message);
    });

    c->connect_failed.connect([this, endpoint_path, endpoint_filename](auto&& error_code) {
      logger::get_logger()->info(
          "client connect_failed: {0}",
          endpoint_filename.c_str());

      erase_client(endpoint_path);
    });

    c->closed.connect([this, endpoint_path, endpoint_filename] {
      logger::get_logger()->info(
          "client closed: {0}",
          endpoint_filename.c_str());

      erase_client(endpoint_path);
    });

    c->async_start();

    entries_[endpoint_path] = std::make_unique<entry>(c,
                                                      run_loop_thread_,
                                                      expected_driver_version);

    logger::get_logger()->info("virtual_hid_device_service_clients_manager ({0}) client is added (size: {1})",
                               endpoint_filename.c_str(),
                               entries_.size());
  }

  // This method needs to be called in the dispatcher thread.
  void erase_client(const std::string& endpoint_path) {
    if (!dispatcher_thread()) {
      throw std::logic_error(fmt::format("{0} is called in wrong thread", __func__));
    }

    auto endpoint_filename = std::filesystem::path(endpoint_path).filename();

    entries_.erase(endpoint_path);

    logger::get_logger()->info("virtual_hid_device_service_clients_manager ({0}) client is removed (size: {1})",
                               endpoint_filename.c_str(),
                               entries_.size());
  }

  void initialize_keyboard(const std::string& endpoint_path,
                           pqrs::hid::country_code::value_t country_code) {
    if (!dispatcher_thread()) {
      throw std::logic_error(fmt::format("{0} is called in wrong thread", __func__));
    }

    auto it = entries_.find(endpoint_path);
    if (it != std::end(entries_)) {
      it->second->initialize_keyboard(country_code);
    }
  }

  void terminate_keyboard(const std::string& endpoint_path) {
    if (!dispatcher_thread()) {
      throw std::logic_error(fmt::format("{0} is called in wrong thread", __func__));
    }

    auto it = entries_.find(endpoint_path);
    if (it != std::end(entries_)) {
      it->second->terminate_keyboard();
    }
  }

  void initialize_pointing(const std::string& endpoint_path) {
    if (!dispatcher_thread()) {
      throw std::logic_error(fmt::format("{0} is called in wrong thread", __func__));
    }

    auto it = entries_.find(endpoint_path);
    if (it != std::end(entries_)) {
      it->second->initialize_pointing();
    }
  }

  void terminate_pointing(const std::string& endpoint_path) {
    if (!dispatcher_thread()) {
      throw std::logic_error(fmt::format("{0} is called in wrong thread", __func__));
    }

    auto it = entries_.find(endpoint_path);
    if (it != std::end(entries_)) {
      it->second->terminate_pointing();
    }
  }

  // This method needs to be called in the dispatcher thread.
  std::optional<bool> virtual_hid_keyboard_ready(const std::string& endpoint_path) const {
    if (!dispatcher_thread()) {
      throw std::logic_error(fmt::format("{0} is called in wrong thread", __func__));
    }

    auto it = entries_.find(endpoint_path);
    if (it != std::end(entries_)) {
      if (auto c = it->second->get_io_service_client_keyboard()) {
        return c->get_virtual_hid_keyboard_ready(it->second->get_expected_driver_version());
      }
    }

    return std::nullopt;
  }

  // This method needs to be called in the dispatcher thread.
  std::optional<bool> virtual_hid_pointing_ready(const std::string& endpoint_path) const {
    if (!dispatcher_thread()) {
      throw std::logic_error(fmt::format("{0} is called in wrong thread", __func__));
    }

    auto it = entries_.find(endpoint_path);
    if (it != std::end(entries_)) {
      if (auto c = it->second->get_io_service_client_pointing()) {
        return c->get_virtual_hid_pointing_ready(it->second->get_expected_driver_version());
      }
    }

    return std::nullopt;
  }

  // This method needs to be called in the dispatcher thread.
  void virtual_hid_keyboard_reset(const std::string& endpoint_path) const {
    if (!dispatcher_thread()) {
      throw std::logic_error(fmt::format("{0} is called in wrong thread", __func__));
    }

    auto it = entries_.find(endpoint_path);
    if (it != std::end(entries_)) {
      if (auto c = it->second->get_io_service_client_keyboard()) {
        c->async_virtual_hid_keyboard_reset(it->second->get_expected_driver_version());
      }
    }
  }

  // This method needs to be called in the dispatcher thread.
  void virtual_hid_pointing_reset(const std::string& endpoint_path) const {
    if (!dispatcher_thread()) {
      throw std::logic_error(fmt::format("{0} is called in wrong thread", __func__));
    }

    auto it = entries_.find(endpoint_path);
    if (it != std::end(entries_)) {
      if (auto c = it->second->get_io_service_client_pointing()) {
        c->async_virtual_hid_pointing_reset(it->second->get_expected_driver_version());
      }
    }
  }

  // This method needs to be called in the dispatcher thread.
  template <typename T>
  void post_keyboard_report(const std::string& endpoint_path,
                            const uint8_t* buffer,
                            size_t buffer_size) const {
    if (!dispatcher_thread()) {
      throw std::logic_error(fmt::format("{0} is called in wrong thread", __func__));
    }

    if (sizeof(T) != buffer_size) {
      logger::get_logger()->warn(fmt::format("{0}: buffer size error", __func__));
      return;
    }

    auto it = entries_.find(endpoint_path);
    if (it != std::end(entries_)) {
      if (auto c = it->second->get_io_service_client_keyboard()) {
        c->async_post_report(
            it->second->get_expected_driver_version(),
            *(reinterpret_cast<const T*>(buffer)));
      }
    }
  }

  // This method needs to be called in the dispatcher thread.
  template <typename T>
  void post_pointing_report(const std::string& endpoint_path,
                            const uint8_t* buffer,
                            size_t buffer_size) const {
    if (!dispatcher_thread()) {
      throw std::logic_error(fmt::format("{0} is called in wrong thread", __func__));
    }

    if (sizeof(T) != buffer_size) {
      logger::get_logger()->warn(fmt::format("{0}: buffer size error", __func__));
      return;
    }

    auto it = entries_.find(endpoint_path);
    if (it != std::end(entries_)) {
      if (auto c = it->second->get_io_service_client_pointing()) {
        c->async_post_report(
            it->second->get_expected_driver_version(),
            *(reinterpret_cast<const T*>(buffer)));
      }
    }
  }

private:
  class entry final : public pqrs::dispatcher::extra::dispatcher_client {
  public:
    entry(std::shared_ptr<pqrs::local_datagram::client> local_datagram_client,
          std::shared_ptr<pqrs::cf::run_loop_thread> run_loop_thread,
          pqrs::karabiner::driverkit::driver_version::value_t expected_driver_version)
        : local_datagram_client_(local_datagram_client),
          run_loop_thread_(run_loop_thread),
          expected_driver_version_(expected_driver_version),
          timer_(*this) {
      timer_.start(
          [this] {
            if (io_service_client_keyboard_) {
              io_service_client_keyboard_->async_virtual_hid_keyboard_ready(expected_driver_version_);
            }
            if (io_service_client_pointing_) {
              io_service_client_pointing_->async_virtual_hid_pointing_ready(expected_driver_version_);
            }
          },
          std::chrono::milliseconds(1000));
    }

    ~entry(void) {
      detach_from_dispatcher([this] {
        io_service_client_keyboard_ = nullptr;
        io_service_client_pointing_ = nullptr;
        local_datagram_client_ = nullptr;
      });
    }

    std::shared_ptr<pqrs::local_datagram::client> get_local_datagram_client(void) const {
      return local_datagram_client_;
    }

    std::shared_ptr<io_service_client> get_io_service_client_keyboard(void) const {
      return io_service_client_keyboard_;
    }

    std::shared_ptr<io_service_client> get_io_service_client_pointing(void) const {
      return io_service_client_pointing_;
    }

    pqrs::karabiner::driverkit::driver_version::value_t get_expected_driver_version(void) const {
      return expected_driver_version_;
    }

    //
    // io_service_client_keyboard_
    //

    void initialize_keyboard(pqrs::hid::country_code::value_t country_code) {
      enqueue_to_dispatcher([this, country_code] {
        logger::get_logger()->info("entry::{0}", __func__);

        io_service_client_keyboard_ = std::make_shared<io_service_client>(run_loop_thread_);

        io_service_client_keyboard_->opened.connect([this, country_code] {
          io_service_client_keyboard_->async_virtual_hid_keyboard_initialize(expected_driver_version_,
                                                                             country_code);
        });

        io_service_client_keyboard_->async_start();
      });
    }

    void terminate_keyboard(void) {
      enqueue_to_dispatcher([this] {
        logger::get_logger()->info("entry::{0}", __func__);

        io_service_client_keyboard_ = nullptr;
      });
    }

    //
    // io_service_client_pointing_
    //

    void initialize_pointing(void) {
      enqueue_to_dispatcher([this] {
        logger::get_logger()->info("entry::{0}", __func__);

        io_service_client_pointing_ = std::make_shared<io_service_client>(run_loop_thread_);

        io_service_client_pointing_->opened.connect([this] {
          io_service_client_pointing_->async_virtual_hid_pointing_initialize(expected_driver_version_);
        });

        io_service_client_pointing_->async_start();
      });
    }

    void terminate_pointing(void) {
      enqueue_to_dispatcher([this] {
        logger::get_logger()->info("entry::{0}", __func__);

        io_service_client_pointing_ = nullptr;
      });
    }

  private:
    std::shared_ptr<pqrs::local_datagram::client> local_datagram_client_;
    std::shared_ptr<pqrs::cf::run_loop_thread> run_loop_thread_;
    pqrs::karabiner::driverkit::driver_version::value_t expected_driver_version_;

    std::shared_ptr<io_service_client> io_service_client_keyboard_;
    std::shared_ptr<io_service_client> io_service_client_pointing_;
    pqrs::dispatcher::extra::timer timer_;
  };

  std::filesystem::path server_response_socket_file_path(void) const {
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();

    std::stringstream ss;
    ss << pqrs::karabiner::driverkit::virtual_hid_device_service::constants::get_server_response_socket_directory_path().string()
       << "/"
       << std::hex
       << std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count()
       << ".sock";

    return ss.str();
  }

  std::shared_ptr<pqrs::cf::run_loop_thread> run_loop_thread_;
  std::unordered_map<std::string, std::unique_ptr<entry>> entries_;
};
