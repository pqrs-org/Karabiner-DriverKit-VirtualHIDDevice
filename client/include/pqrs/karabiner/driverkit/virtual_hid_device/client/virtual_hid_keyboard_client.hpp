#pragma once

#include <IOKit/IOKitLib.h>
#include <os/log.h>
#include <pqrs/karabiner/driverkit/virtual_hid_device.hpp>

namespace pqrs {
namespace karabiner {
namespace driverkit {
namespace virtual_hid_device {
namespace client {

class virtual_hid_keyboard_client final {
public:
  virtual_hid_keyboard_client(void) : connection_(IO_OBJECT_NULL) {
    if (auto service = IOServiceGetMatchingService(kIOMasterPortDefault, IOServiceNameMatching("org_pqrs_KarabinerDriverKitVirtualHIDKeyboard"))) {
      auto kr = IOServiceOpen(service, mach_task_self(), 0, &connection_);
      if (kr != kIOReturnSuccess) {
        os_log_error(OS_LOG_DEFAULT, "IOServiceOpen error: 0x%x", kr);
        connection_ = IO_OBJECT_NULL;
      }

      IOObjectRelease(service);
    }
  }

  ~virtual_hid_keyboard_client(void) {
    if (connection_) {
      reset();

      IOServiceClose(connection_);
    }
  }

  bool connected(void) const {
    return connection_;
  }

  kern_return_t post_report(const void* report,
                            size_t report_size) const {
    if (!connection_) {
      return kIOReturnNotOpen;
    }

    return IOConnectCallStructMethod(connection_,
                                     static_cast<uint32_t>(virtual_hid_device::user_client_method::virtual_hid_keyboard_post_report),
                                     report,
                                     report_size,
                                     nullptr,
                                     0);
  }

  kern_return_t post_report(const virtual_hid_device::hid_report::keyboard_input& report) const {
    return post_report(&report, sizeof(report));
  }

  kern_return_t post_report(const virtual_hid_device::hid_report::consumer_input& report) const {
    return post_report(&report, sizeof(report));
  }

  kern_return_t post_report(const virtual_hid_device::hid_report::apple_vendor_keyboard_input& report) const {
    return post_report(&report, sizeof(report));
  }

  kern_return_t post_report(const virtual_hid_device::hid_report::apple_vendor_top_case_input& report) const {
    return post_report(&report, sizeof(report));
  }

  kern_return_t reset(void) const {
    if (!connection_) {
      return kIOReturnNotOpen;
    }

    return IOConnectCallStructMethod(connection_,
                                     static_cast<uint32_t>(virtual_hid_device::user_client_method::virtual_hid_keyboard_reset),
                                     nullptr,
                                     0,
                                     nullptr,
                                     0);
  }

private:
  io_connect_t connection_;
};

} // namespace client
} // namespace virtual_hid_device
} // namespace driverkit
} // namespace karabiner
} // namespace pqrs
