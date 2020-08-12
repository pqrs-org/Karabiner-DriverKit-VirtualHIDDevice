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

  pqrs::dispatcher::extra::initialize_shared_dispatcher();

  auto client = std::make_shared<io_service_client>();

  client->service_opened.connect([client] {
    std::cout << "service opened" << std::endl;

    pqrs::osx::iokit_return r = client->virtual_hid_keyboard_initialize(0);
    std::cout << r << std::endl;

    while (true) {
      if (auto ready = client->virtual_hid_keyboard_ready()) {
        std::cout << "ready " << *ready << std::endl;

        if (*ready) {
          break;
        }
      }

      std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    std::cout << "post" << std::endl;

    {
      // key down
      {
        pqrs::karabiner::driverkit::virtual_hid_device::hid_report::apple_vendor_keyboard_input report;
        report.keys.insert(type_safe::get(pqrs::hid::usage::apple_vendor_keyboard::launchpad));
        r = client->post_report(report);
        std::cout << r << std::endl;
      }
      // key up
      {
        pqrs::karabiner::driverkit::virtual_hid_device::hid_report::apple_vendor_keyboard_input report;
        r = client->post_report(report);
        std::cout << r << std::endl;
      }
    }
  });

  client->async_open();

  global_wait->wait_notice();

  pqrs::dispatcher::extra::terminate_shared_dispatcher();

  return 0;
}
