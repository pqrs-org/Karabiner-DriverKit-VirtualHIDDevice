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
      clients_.clear();
    });
  }

  // This method needs to be called inside the dispatcher thread.
  void insert_client(const std::string& endpoint_path) {
    if (clients_.contains(endpoint_path)) {
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

    clients_[endpoint_path] = c;

    logger::get_logger()->info("virtual_hid_device_service_clients_manager ({0}) client is added (size: {1})",
                               name_,
                               clients_.size());
  }

private:
  // This method is executed in the dispatcher thread.
  void erase_client(const std::string& endpoint_path) {
    if (clients_.empty()) {
      return;
    }

    clients_.erase(endpoint_path);

    logger::get_logger()->info("virtual_hid_device_service_clients_manager ({0}) client is removed (size: {1})",
                               name_,
                               clients_.size());

    if (clients_.empty()) {
      enqueue_to_dispatcher([this] {
        all_clients_disconnected();
      });
    }
  }

private:
  std::string name_;
  std::unordered_map<std::string, std::shared_ptr<pqrs::local_datagram::client>> clients_;
};
