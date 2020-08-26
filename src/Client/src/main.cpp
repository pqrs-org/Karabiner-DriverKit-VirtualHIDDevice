#include "io_service_client.hpp"
#include <chrono>
#include <iostream>
#include <memory>
#include <pqrs/hid.hpp>
#include <pqrs/local_datagram.hpp>
#include <pqrs/osx/iokit_return.hpp>
#include <thread>

namespace {
auto global_wait = pqrs::make_thread_wait();
}

int main(void) {
  std::signal(SIGINT, [](int) {
    global_wait->notify();
  });

  logger::set_async_rotating_logger("karabiner_driverkit_virtual_hid_device_service",
                                    "/var/log/karabiner_driverkit_virtual_hid_device_service.log",
                                    pqrs::spdlog::filesystem::log_directory_perms_0700);

  pqrs::dispatcher::extra::initialize_shared_dispatcher();

  std::optional<bool> virtual_hid_keyboard_ready;
  std::mutex virtual_hid_keyboard_ready_mutex;

  auto client = std::make_shared<io_service_client>();
  client->virtual_hid_keyboard_ready_callback.connect([&virtual_hid_keyboard_ready, &virtual_hid_keyboard_ready_mutex](auto&& ready) {
    std::lock_guard<std::mutex> lock(virtual_hid_keyboard_ready_mutex);

    virtual_hid_keyboard_ready = ready;

    if (ready) {
      std::cout << "ready " << *ready << std::endl;
    } else {
      std::cout << "ready std::nullopt" << std::endl;
    }
  });
  client->async_start();

  std::this_thread::sleep_for(std::chrono::milliseconds(1000));

  std::cout << "initialize" << std::endl;

  client->async_virtual_hid_keyboard_initialize(pqrs::hid::country_code::us);

  while (true) {
    client->async_virtual_hid_keyboard_ready();
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    {
      std::lock_guard<std::mutex> lock(virtual_hid_keyboard_ready_mutex);

      if (virtual_hid_keyboard_ready && *virtual_hid_keyboard_ready) {
        break;
      }
    }
  }

  std::cout << "post" << std::endl;

  {
    // key down
    {
      pqrs::karabiner::driverkit::virtual_hid_device::hid_report::apple_vendor_keyboard_input report;
      report.keys.insert(type_safe::get(pqrs::hid::usage::apple_vendor_keyboard::launchpad));
      client->async_post_report(report);
    }
    // key up
    {
      pqrs::karabiner::driverkit::virtual_hid_device::hid_report::apple_vendor_keyboard_input report;
      client->async_post_report(report);
    }
  }

  global_wait->wait_notice();

  pqrs::dispatcher::extra::terminate_shared_dispatcher();

  return 0;
}
