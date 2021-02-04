#pragma once

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

  virtual_hid_device_service_clients_manager(void) : dispatcher_client() {
  }

  virtual ~virtual_hid_device_service_clients_manager(void) {
    detach_from_dispatcher([this] {
      clients_.clear();
    });
  }

  void async_insert_client(const std::string& endpoint_path) {
    enqueue_to_dispatcher([this, endpoint_path] {
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
    });
  }

private:
  // This method is executed in the dispatcher thread.
  void erase_client(const std::string& endpoint_path) {
    if (clients_.empty()) {
      return;
    }

    clients_.erase(endpoint_path);

    if (clients_.empty()) {
      enqueue_to_dispatcher([this] {
        all_clients_disconnected();
      });
    }
  }

private:
  std::unordered_map<std::string, std::shared_ptr<pqrs::local_datagram::client>> clients_;
};
