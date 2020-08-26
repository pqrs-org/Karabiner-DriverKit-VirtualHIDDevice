#include "client.h"
#include <cmath>
#include <memory>
#include <os/log.h>
#include <pqrs/hid.hpp>
#include <pqrs/karabiner/driverkit/virtual_hid_device/client/virtual_hid_keyboard_client.hpp>
#include <thread>

namespace {
std::shared_ptr<pqrs::karabiner::driverkit::virtual_hid_device::client::virtual_hid_keyboard_client> client;
}

void shared_virtual_hid_keyboard_client_initialize(void) {
  if (!client) {
    client = std::make_shared<pqrs::karabiner::driverkit::virtual_hid_device::client::virtual_hid_keyboard_client>();
  }
}

void shared_virtual_hid_keyboard_client_terminate(void) {
  client = nullptr;
}

int shared_virtual_hid_keyboard_client_connected(void) {
  if (client) {
    return client->connected();
  }

  return 0;
}

void shared_virtual_hid_keyboard_initialize(uint32_t country_code) {
  if (client) {
    client->virtual_hid_keyboard_initialize(country_code);
  }
}

void shared_virtual_hid_keyboard_terminate(void) {
  if (client) {
    client->virtual_hid_keyboard_terminate();
  }
}

int shared_virtual_hid_keyboard_ready(void) {
  if (client) {
    auto ready = client->virtual_hid_keyboard_ready();
    if (ready) {
      return *ready;
    }
  }

  return -1;
}

void shared_virtual_hid_keyboard_post_control_up(void) {
  if (!client) {
    return;
  }

  // key down
  {
    pqrs::karabiner::driverkit::virtual_hid_device_driver::hid_report::keyboard_input report;
    report.modifiers.insert(pqrs::karabiner::driverkit::virtual_hid_device_driver::hid_report::modifier::left_control);
    report.keys.insert(type_safe::get(pqrs::hid::usage::keyboard_or_keypad::keyboard_up_arrow));
    client->post_report(report);
  }

  // key up
  {
    pqrs::karabiner::driverkit::virtual_hid_device_driver::hid_report::keyboard_input report;
    client->post_report(report);
  }
}

void shared_virtual_hid_keyboard_post_launchpad(void) {
  if (!client) {
    return;
  }

  // key down
  {
    pqrs::karabiner::driverkit::virtual_hid_device_driver::hid_report::apple_vendor_keyboard_input report;
    report.keys.insert(type_safe::get(pqrs::hid::usage::apple_vendor_keyboard::launchpad));
    client->post_report(report);
  }
  // key up
  {
    pqrs::karabiner::driverkit::virtual_hid_device_driver::hid_report::apple_vendor_keyboard_input report;
    client->post_report(report);
  }
}

void shared_virtual_hid_keyboard_post_fn(void) {
  if (!client) {
    return;
  }

  // key down
  {
    pqrs::karabiner::driverkit::virtual_hid_device_driver::hid_report::apple_vendor_top_case_input report;
    report.keys.insert(type_safe::get(pqrs::hid::usage::apple_vendor_top_case::keyboard_fn));
    client->post_report(report);
  }

  std::this_thread::sleep_for(std::chrono::milliseconds(500));

  // key up
  {
    pqrs::karabiner::driverkit::virtual_hid_device_driver::hid_report::apple_vendor_top_case_input report;
    client->post_report(report);
  }
}

void shared_virtual_hid_keyboard_reset(void) {
  if (client) {
    client->virtual_hid_keyboard_reset();
  }
}

void shared_virtual_hid_pointing_initialize(void) {
  if (client) {
    client->virtual_hid_pointing_initialize();
  }
}

void shared_virtual_hid_pointing_terminate(void) {
  if (client) {
    client->virtual_hid_pointing_terminate();
  }
}

int shared_virtual_hid_pointing_ready(void) {
  if (client) {
    auto ready = client->virtual_hid_pointing_ready();
    if (ready) {
      return *ready;
    }
  }

  return -1;
}

void shared_virtual_hid_pointing_post_example_report(void) {
  if (client) {
    for (int i = 0; i < 400; ++i) {
      pqrs::karabiner::driverkit::virtual_hid_device_driver::hid_report::pointing_input report;
      report.x = static_cast<uint8_t>(cos(0.1 * i) * 20);
      report.y = static_cast<uint8_t>(sin(0.1 * i) * 20);
      client->post_report(report);

      std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
  }
}

void shared_virtual_hid_pointing_reset(void) {
  if (client) {
    client->virtual_hid_pointing_reset();
  }
}
