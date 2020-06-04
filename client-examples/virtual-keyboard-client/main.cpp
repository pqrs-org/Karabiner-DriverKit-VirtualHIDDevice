#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/hid/IOHIDDevice.h>
#include <IOKit/hid/IOHIDElement.h>
#include <IOKit/hid/IOHIDManager.h>
#include <IOKit/hid/IOHIDQueue.h>
#include <IOKit/hid/IOHIDValue.h>
#include <IOKit/hidsystem/IOHIDShared.h>
#include <IOKit/hidsystem/ev_keymap.h>
#include <cmath>
#include <iostream>
#include <pqrs/hid.hpp>
#include <pqrs/karabiner/driverkit/virtual_hid_keyboard_client.hpp>
#include <pqrs/osx/iokit_return.hpp>
#include <thread>

int main(int argc, const char* argv[]) {
  auto client = std::make_shared<pqrs::karabiner::driverkit::virtual_hid_keyboard_client>();

  if (!client->connected()) {
    std::cerr << "virtual_hid_keyboard is not connected" << std::endl;
  } else {
    //
    // keyboard_input
    //

    {
      pqrs::karabiner::driverkit::virtual_hid_device::hid_report::keyboard_input report;

      report.modifiers.insert(pqrs::karabiner::driverkit::virtual_hid_device::hid_report::modifier::left_command);
      report.keys.insert(type_safe::get(pqrs::hid::usage::keyboard_or_keypad::keyboard_tab));

      pqrs::osx::iokit_return kr = client->post_keyboard_input_report(report);
      if (!kr) {
        std::cerr << "post_keyboard_input_report error: " << kr << std::endl;
      }

      kr = client->reset_virtual_hid_keyboard();
      if (!kr) {
        std::cerr << "reset_virtual_hid_keyboard error: " << kr << std::endl;
      }
      std::cout << kr << std::endl;

      return 0;

      report.modifiers.clear();
      report.keys.clear();

      kr = client->post_keyboard_input_report(report);
      if (!kr) {
        std::cerr << "post_keyboard_input_report error: " << kr << std::endl;
      }
    }

    {
      pqrs::karabiner::driverkit::virtual_hid_device::hid_report::apple_vendor_keyboard_input report;

      report.keys.insert(type_safe::get(pqrs::hid::usage::apple_vendor_keyboard::expose_all));

      pqrs::osx::iokit_return kr = client->post_keyboard_input_report(report);
      if (!kr) {
        std::cerr << "post_keyboard_input_report error: " << kr << std::endl;
      }

      report.keys.clear();

      kr = client->post_keyboard_input_report(report);
      if (!kr) {
        std::cerr << "post_keyboard_input_report error: " << kr << std::endl;
      }
    }
  }

  client = nullptr;

  return 0;
}
