#pragma once

#include <IOKit/IOKitLib.h>
#include <array>
#include <optional>
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
    if (auto service = IOServiceGetMatchingService(kIOMasterPortDefault, IOServiceNameMatching("org_pqrs_Karabiner_DriverKit_VirtualHIDDeviceRoot"))) {
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
      IOServiceClose(connection_);
    }
  }

  bool connected(void) const {
    return connection_;
  }

  kern_return_t virtual_hid_keyboard_initialize(uint32_t country_code) const {
    std::array<uint64_t, 1> input = {country_code};

    return call_scalar_method(virtual_hid_device::user_client_method::virtual_hid_keyboard_initialize,
                              input.data(),
                              input.size());
  }

  kern_return_t virtual_hid_keyboard_terminate(void) const {
    return call(virtual_hid_device::user_client_method::virtual_hid_keyboard_terminate);
  }

  std::optional<bool> virtual_hid_keyboard_ready(void) const {
    return call_ready(virtual_hid_device::user_client_method::virtual_hid_keyboard_ready);
  }

  kern_return_t virtual_hid_keyboard_reset(void) const {
    return call(virtual_hid_device::user_client_method::virtual_hid_keyboard_reset);
  }

  kern_return_t virtual_hid_pointing_initialize(void) const {
    return call(virtual_hid_device::user_client_method::virtual_hid_pointing_initialize);
  }

  kern_return_t virtual_hid_pointing_terminate(void) const {
    return call(virtual_hid_device::user_client_method::virtual_hid_pointing_terminate);
  }

  std::optional<bool> virtual_hid_pointing_ready(void) const {
    return call_ready(virtual_hid_device::user_client_method::virtual_hid_pointing_ready);
  }

  kern_return_t virtual_hid_pointing_reset(void) const {
    return call(virtual_hid_device::user_client_method::virtual_hid_pointing_reset);
  }

  kern_return_t post_report(const virtual_hid_device::hid_report::keyboard_input& report) const {
    return post_report(
        virtual_hid_device::user_client_method::virtual_hid_keyboard_post_report,
        &report,
        sizeof(report));
  }

  kern_return_t post_report(const virtual_hid_device::hid_report::consumer_input& report) const {
    return post_report(
        virtual_hid_device::user_client_method::virtual_hid_keyboard_post_report,
        &report,
        sizeof(report));
  }

  kern_return_t post_report(const virtual_hid_device::hid_report::apple_vendor_keyboard_input& report) const {
    return post_report(
        virtual_hid_device::user_client_method::virtual_hid_keyboard_post_report,
        &report,
        sizeof(report));
  }

  kern_return_t post_report(const virtual_hid_device::hid_report::apple_vendor_top_case_input& report) const {
    return post_report(
        virtual_hid_device::user_client_method::virtual_hid_keyboard_post_report,
        &report,
        sizeof(report));
  }

  kern_return_t post_report(const virtual_hid_device::hid_report::pointing_input& report) const {
    return post_report(
        virtual_hid_device::user_client_method::virtual_hid_pointing_post_report,
        &report,
        sizeof(report));
  }

private:
  kern_return_t call(virtual_hid_device::user_client_method user_client_method) const {
    if (!connection_) {
      return kIOReturnNotOpen;
    }

    return IOConnectCallStructMethod(connection_,
                                     static_cast<uint32_t>(user_client_method),
                                     nullptr,
                                     0,
                                     nullptr,
                                     0);
  }

  kern_return_t call_scalar_method(virtual_hid_device::user_client_method user_client_method,
                                   const uint64_t* input,
                                   uint32_t input_count) const {
    if (!connection_) {
      return kIOReturnNotOpen;
    }

    return IOConnectCallScalarMethod(connection_,
                                     static_cast<uint32_t>(user_client_method),
                                     input,
                                     input_count,
                                     nullptr,
                                     0);
  }

  std::optional<bool> call_ready(virtual_hid_device::user_client_method user_client_method) const {
    if (!connection_) {
      return std::nullopt;
    }

    uint64_t output[1] = {0};
    uint32_t output_count = 1;
    auto kr = IOConnectCallScalarMethod(connection_,
                                        static_cast<uint32_t>(user_client_method),
                                        nullptr,
                                        0,
                                        output,
                                        &output_count);

    if (kr != kIOReturnSuccess) {
      return std::nullopt;
    }

    return output[0];
  }

  kern_return_t post_report(virtual_hid_device::user_client_method user_client_method,
                            const void* report,
                            size_t report_size) const {
    if (!connection_) {
      return kIOReturnNotOpen;
    }

    return IOConnectCallStructMethod(connection_,
                                     static_cast<uint32_t>(user_client_method),
                                     report,
                                     report_size,
                                     nullptr,
                                     0);
  }

  io_connect_t connection_;
};

} // namespace client
} // namespace virtual_hid_device
} // namespace driverkit
} // namespace karabiner
} // namespace pqrs
