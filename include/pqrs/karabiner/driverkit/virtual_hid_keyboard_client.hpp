#pragma once

#include "virtual_hid_device.hpp"
#include <os/log.h>

namespace pqrs {
namespace karabiner {
namespace driverkit {

class virtual_hid_keyboard_client final {
public:
  virtual_hid_keyboard_client(void) : connection_(IO_OBJECT_NULL) {
    if (auto service = IOServiceGetMatchingService(kIOMasterPortDefault, IOServiceNameMatching("org_pqrs_KarabinerDriverKitVirtualHIDKeyboard"))) {
      auto kr = IOServiceOpen(service, mach_task_self(), kIOHIDServerConnectType, &connection_);
      if (kr != kIOReturnSuccess) {
        os_log_error(OS_LOG_DEFAULT, "IOServiceOpen error: 0x%x", kr);
        connection_ = IO_OBJECT_NULL;
      }

      IOObjectRelease(service);
    }
  }

  ~virtual_hid_keyboard_client(void) {
    if (connection_) {
      IOServiceClose(connection_);
    }
  }

  bool connected(void) const {
    return connection_;
  }

  kern_return_t post_keyboard_input_report(const virtual_hid_device::hid_report::keyboard_input& report) const {
    if (!connection_) {
      return kIOReturnNotOpen;
    }

    return IOConnectCallStructMethod(connection_,
                                     static_cast<uint32_t>(virtual_hid_device::user_client_method::post_keyboard_input_report),
                                     &report,
                                     sizeof(report),
                                     nullptr,
                                     0);
  }

private:
  io_connect_t connection_;
};

} // namespace driverkit
} // namespace karabiner
} // namespace pqrs
