#pragma once

#include "logger.hpp"
#include <nod/nod.hpp>
#include <pqrs/dispatcher.hpp>
#include <pqrs/karabiner/driverkit/virtual_hid_device_service.hpp>
#include <pqrs/local_datagram.hpp>
#include <unordered_map>

class virtual_hid_device_service_clients_manager final : public pqrs::dispatcher::extra::dispatcher_client {
public:
  // Signals (invoked from the dispatcher thread)

  nod::signal<void(void)> all_clients_disconnected;

  // Methods

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
                     pqrs::karabiner::driverkit::driver_version::value_t expected_driver_version) {
    if (!dispatcher_thread()) {
      throw std::logic_error("virtual_hid_device_service_clients_manager::insert_client is called in wrong thread");
    }

    if (entries_.contains(endpoint_path)) {
      return;
    }

    auto c = std::make_shared<pqrs::local_datagram::client>(
        weak_dispatcher_,
        endpoint_path,
        std::nullopt,
        pqrs::karabiner::driverkit::virtual_hid_device_service::constants::local_datagram_buffer_size);

    c->set_server_check_interval(std::chrono::milliseconds(1000));

    c->connect_failed.connect([this, endpoint_path](auto&& error_code) {
      erase_client(endpoint_path);
    });

    c->closed.connect([this, endpoint_path] {
      erase_client(endpoint_path);
    });

    c->async_start();

    entries_[endpoint_path] = std::make_unique<entry>(c,
                                                      expected_driver_version);

    logger::get_logger()->info("virtual_hid_device_service_clients_manager ({0}) client is added (size: {1})",
                               name_,
                               entries_.size());
  }

private:
  // This method is executed in the dispatcher thread.
  void erase_client(const std::string& endpoint_path) {
    if (entries_.empty()) {
      return;
    }

    entries_.erase(endpoint_path);

    logger::get_logger()->info("virtual_hid_device_service_clients_manager ({0}) client is removed (size: {1})",
                               name_,
                               entries_.size());

    if (entries_.empty()) {
      enqueue_to_dispatcher([this] {
        all_clients_disconnected();
      });
    }
  }

private:
  class entry final {
  public:
    entry(std::shared_ptr<pqrs::local_datagram::client> client,
          pqrs::karabiner::driverkit::driver_version::value_t expected_driver_version)
        : client_(client),
          expected_driver_version_(expected_driver_version) {
    }

    std::shared_ptr<pqrs::local_datagram::client> get_client(void) const {
      return client_;
    }

    pqrs::karabiner::driverkit::driver_version::value_t get_expected_driver_version(void) const {
      return expected_driver_version_;
    }

  private:
    std::shared_ptr<pqrs::local_datagram::client> client_;
    pqrs::karabiner::driverkit::driver_version::value_t expected_driver_version_;
  };

  std::string name_;
  std::unordered_map<std::string, std::unique_ptr<entry>> entries_;
};
