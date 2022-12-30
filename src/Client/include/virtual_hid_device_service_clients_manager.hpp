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
  virtual_hid_device_service_clients_manager(const std::string& name) : dispatcher_client(),
                                                                        name_(name) {
  }

  virtual ~virtual_hid_device_service_clients_manager(void) {
    detach_from_dispatcher([this] {
      entries_.clear();
    });
  }

  // This method needs to be called in the dispatcher thread.
  void insert_client(const std::string& endpoint_path,
                     std::shared_ptr<io_service_client> io_service_client,
                     pqrs::karabiner::driverkit::driver_version::value_t expected_driver_version) {
    if (!dispatcher_thread()) {
      throw std::logic_error(fmt::format("{0} is called in wrong thread", __func__));
    }

    auto endpoint_filename = std::filesystem::path(endpoint_path).filename();

    //
    // Create pqrs::local_datagram::client
    //

    logger::get_logger()->info(
        "create a client for virtual_hid_device_service::client: {0} {1}",
        name_,
        endpoint_filename.c_str());

    auto c = std::make_shared<pqrs::local_datagram::client>(
        weak_dispatcher_,
        endpoint_path,
        std::nullopt,
        pqrs::karabiner::driverkit::virtual_hid_device_service::constants::local_datagram_buffer_size);

    c->set_server_check_interval(std::chrono::milliseconds(1000));
    c->set_next_heartbeat_deadline(std::chrono::milliseconds(3000));

    c->warning_reported.connect([this, endpoint_filename](auto&& message) {
      logger::get_logger()->warn(
          "client: {0} {1} {2}",
          name_,
          endpoint_filename.c_str(),
          message);
    });

    c->connect_failed.connect([this, endpoint_path, endpoint_filename](auto&& error_code) {
      logger::get_logger()->info(
          "client connect_failed: {0} {1}",
          name_,
          endpoint_filename.c_str());

      erase_client(endpoint_path);
    });

    c->closed.connect([this, endpoint_path, endpoint_filename] {
      logger::get_logger()->info(
          "client closed: {0} {1}",
          name_,
          endpoint_filename.c_str());

      erase_client(endpoint_path);
    });

    c->async_start();

    entries_[endpoint_path] = std::make_unique<entry>(c,
                                                      io_service_client,
                                                      expected_driver_version);

    logger::get_logger()->info("virtual_hid_device_service_clients_manager ({0} {1}) client is added (size: {2})",
                               name_,
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

    logger::get_logger()->info("virtual_hid_device_service_clients_manager ({0} {1}) client is removed (size: {2})",
                               name_,
                               endpoint_filename.c_str(),
                               entries_.size());
  }

  // This method needs to be called in the dispatcher thread.
  void call_async_virtual_hid_keyboard_ready(void) const {
    if (!dispatcher_thread()) {
      throw std::logic_error(fmt::format("{0} is called in wrong thread", __func__));
    }

    for (const auto& [k, v] : entries_) {
      if (auto c = v->get_io_service_client()) {
        c->async_virtual_hid_keyboard_ready(v->get_expected_driver_version());
      }
    }
  }

  // This method needs to be called in the dispatcher thread.
  void call_async_virtual_hid_pointing_ready(void) const {
    if (!dispatcher_thread()) {
      throw std::logic_error(fmt::format("{0} is called in wrong thread", __func__));
    }

    for (const auto& [k, v] : entries_) {
      if (auto c = v->get_io_service_client()) {
        c->async_virtual_hid_pointing_ready(v->get_expected_driver_version());
      }
    }
  }

  // This method needs to be called in the dispatcher thread.
  std::optional<bool> virtual_hid_keyboard_ready(const std::string& endpoint_path) const {
    if (!dispatcher_thread()) {
      throw std::logic_error(fmt::format("{0} is called in wrong thread", __func__));
    }

    auto it = entries_.find(endpoint_path);
    if (it != std::end(entries_)) {
      if (auto c = it->second->get_io_service_client()) {
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
      if (auto c = it->second->get_io_service_client()) {
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
      if (auto c = it->second->get_io_service_client()) {
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
      if (auto c = it->second->get_io_service_client()) {
        c->async_virtual_hid_pointing_reset(it->second->get_expected_driver_version());
      }
    }
  }

  // This method needs to be called in the dispatcher thread.
  template <typename T>
  void post_report(const std::string& endpoint_path,
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
      if (auto c = it->second->get_io_service_client()) {
        c->async_post_report(
            it->second->get_expected_driver_version(),
            *(reinterpret_cast<const T*>(buffer)));
      }
    }
  }

private:
  class entry final {
  public:
    entry(std::shared_ptr<pqrs::local_datagram::client> local_datagram_client,
          std::shared_ptr<io_service_client> io_service_client,
          pqrs::karabiner::driverkit::driver_version::value_t expected_driver_version)
        : local_datagram_client_(local_datagram_client),
          io_service_client_(io_service_client),
          expected_driver_version_(expected_driver_version) {
    }

    std::shared_ptr<pqrs::local_datagram::client> get_local_datagram_client(void) const {
      return local_datagram_client_;
    }

    std::shared_ptr<io_service_client> get_io_service_client(void) const {
      return io_service_client_;
    }

    pqrs::karabiner::driverkit::driver_version::value_t get_expected_driver_version(void) const {
      return expected_driver_version_;
    }

  private:
    std::shared_ptr<pqrs::local_datagram::client> local_datagram_client_;
    std::shared_ptr<io_service_client> io_service_client_;
    pqrs::karabiner::driverkit::driver_version::value_t expected_driver_version_;
  };

  std::string name_;
  std::unordered_map<std::string, std::unique_ptr<entry>> entries_;
};
