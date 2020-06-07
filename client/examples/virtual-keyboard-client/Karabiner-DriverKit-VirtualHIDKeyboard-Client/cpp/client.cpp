#include "client.h"
#include <memory>
#include <pqrs/hid.hpp>
#include <pqrs/karabiner/driverkit/virtual_hid_device/client/virtual_hid_keyboard_client.hpp>

namespace {
std::shared_ptr<pqrs::karabiner::driverkit::virtual_hid_device::client::virtual_hid_keyboard_client> virtual_hid_keyboard_client;
}

void shared_virtual_hid_keyboard_client_initialize(void) {
  if (!virtual_hid_keyboard_client) {
    virtual_hid_keyboard_client = std::make_shared<pqrs::karabiner::driverkit::virtual_hid_device::client::virtual_hid_keyboard_client>();
  }
}

void shared_virtual_hid_keyboard_client_terminate(void) {
  virtual_hid_keyboard_client = nullptr;
}

int shared_virtual_hid_keyboard_client_connected(void) {
  if (virtual_hid_keyboard_client) {
    return virtual_hid_keyboard_client->connected();
  }

  return 0;
}

void shared_virtual_hid_keyboard_client_post_control_up(void) {
  if (!virtual_hid_keyboard_client) {
    return;
  }

  // key down
  {
    pqrs::karabiner::driverkit::virtual_hid_device::hid_report::keyboard_input report;
    report.modifiers.insert(pqrs::karabiner::driverkit::virtual_hid_device::hid_report::modifier::left_control);
    report.keys.insert(type_safe::get(pqrs::hid::usage::keyboard_or_keypad::keyboard_up_arrow));
    virtual_hid_keyboard_client->post_report(report);
  }

  // key up
  {
    pqrs::karabiner::driverkit::virtual_hid_device::hid_report::keyboard_input report;
    virtual_hid_keyboard_client->post_report(report);
  }
}

void shared_virtual_hid_keyboard_client_post_launchpad(void) {
  if (!virtual_hid_keyboard_client) {
    return;
  }

  // key down
  {
    pqrs::karabiner::driverkit::virtual_hid_device::hid_report::apple_vendor_keyboard_input report;
    report.keys.insert(type_safe::get(pqrs::hid::usage::apple_vendor_keyboard::launchpad));
    virtual_hid_keyboard_client->post_report(report);
  }
  // key up
  {
    pqrs::karabiner::driverkit::virtual_hid_device::hid_report::apple_vendor_keyboard_input report;
     virtual_hid_keyboard_client->post_report(report);
  }
}

void shared_virtual_hid_keyboard_client_reset(void) {
  if (virtual_hid_keyboard_client) {
    virtual_hid_keyboard_client->reset();
  }
}
