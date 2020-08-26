#pragma once

#include "logger.hpp"
#include <filesystem>
#include <pqrs/dispatcher.hpp>
#include <pqrs/karabiner/driverkit/virtual_hid_device_driver.hpp>
#include <pqrs/karabiner/driverkit/virtual_hid_device_service.hpp>
#include <pqrs/local_datagram.hpp>

class virtual_hid_device_service_server final : public pqrs::dispatcher::extra::dispatcher_client {
public:
  virtual_hid_device_service_server(void) : dispatcher_client() {
    remove_server_socket_file();
  }

  virtual ~virtual_hid_device_service_server(void) {
    detach_from_dispatcher([this] {
      server_ = nullptr;
    });

    logger::get_logger()->info("virtual_hid_device_service_server is terminated");
  }

private:
  void remove_server_socket_file(void) const {
    std::error_code error_code;
    std::filesystem::remove(
        pqrs::karabiner::driverkit::virtual_hid_device_service::constants::server_socket_file_path,
        error_code);
  }

  std::shared_ptr<io_service_client> io_service_client_;
  std::unique_ptr<pqrs::local_datagram::server> server_;
};
