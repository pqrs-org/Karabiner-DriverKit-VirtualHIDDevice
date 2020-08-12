#pragma once

#include <IOKit/IOKitLib.h>
#include <array>
#include <nod/nod.hpp>
#include <optional>
#include <os/log.h>
#include <pqrs/dispatcher.hpp>
#include <pqrs/karabiner/driverkit/virtual_hid_device.hpp>
#include <pqrs/osx/iokit_return.hpp>

class io_service_client final : public pqrs::dispatcher::extra::dispatcher_client {
public:
  // Signals (invoked from the shared dispatcher thread)

  nod::signal<void(void)> service_opened;

  // Methods

  io_service_client(void) : dispatcher_client(),
                            connection_(IO_OBJECT_NULL) {
  }

  ~io_service_client(void) {
    detach_from_dispatcher([this] {
      if (connection_) {
        IOServiceClose(connection_);
      }
    });
  }

  void async_open(void) {
    enqueue_to_dispatcher([this] {
      if (auto service = IOServiceGetMatchingService(kIOMasterPortDefault, IOServiceNameMatching("org_pqrs_Karabiner_DriverKit_VirtualHIDDeviceRoot"))) {
        pqrs::osx::iokit_return r = IOServiceOpen(service, mach_task_self(), 0, &connection_);

        if (!r) {
          os_log_error(OS_LOG_DEFAULT, "IOServiceOpen error: %{public}s", r.to_string().c_str());
          connection_ = IO_OBJECT_NULL;
        } else {
          service_opened();
        }

        IOObjectRelease(service);
      }
    });
  }

  kern_return_t virtual_hid_keyboard_initialize(uint32_t country_code) const {
    std::array<uint64_t, 1> input = {country_code};

    return call_scalar_method(pqrs::karabiner::driverkit::virtual_hid_device::user_client_method::virtual_hid_keyboard_initialize,
                              input.data(),
                              input.size());
  }

  kern_return_t virtual_hid_keyboard_terminate(void) const {
    return call(pqrs::karabiner::driverkit::virtual_hid_device::user_client_method::virtual_hid_keyboard_terminate);
  }

  std::optional<bool> virtual_hid_keyboard_ready(void) const {
    return call_ready(pqrs::karabiner::driverkit::virtual_hid_device::user_client_method::virtual_hid_keyboard_ready);
  }

  kern_return_t virtual_hid_keyboard_reset(void) const {
    return call(pqrs::karabiner::driverkit::virtual_hid_device::user_client_method::virtual_hid_keyboard_reset);
  }

  kern_return_t virtual_hid_pointing_initialize(void) const {
    return call(pqrs::karabiner::driverkit::virtual_hid_device::user_client_method::virtual_hid_pointing_initialize);
  }

  kern_return_t virtual_hid_pointing_terminate(void) const {
    return call(pqrs::karabiner::driverkit::virtual_hid_device::user_client_method::virtual_hid_pointing_terminate);
  }

  std::optional<bool> virtual_hid_pointing_ready(void) const {
    return call_ready(pqrs::karabiner::driverkit::virtual_hid_device::user_client_method::virtual_hid_pointing_ready);
  }

  kern_return_t virtual_hid_pointing_reset(void) const {
    return call(pqrs::karabiner::driverkit::virtual_hid_device::user_client_method::virtual_hid_pointing_reset);
  }

  kern_return_t post_report(const pqrs::karabiner::driverkit::virtual_hid_device::hid_report::keyboard_input& report) const {
    return post_report(
        pqrs::karabiner::driverkit::virtual_hid_device::user_client_method::virtual_hid_keyboard_post_report,
        &report,
        sizeof(report));
  }

  kern_return_t post_report(const pqrs::karabiner::driverkit::virtual_hid_device::hid_report::consumer_input& report) const {
    return post_report(
        pqrs::karabiner::driverkit::virtual_hid_device::user_client_method::virtual_hid_keyboard_post_report,
        &report,
        sizeof(report));
  }

  kern_return_t post_report(const pqrs::karabiner::driverkit::virtual_hid_device::hid_report::apple_vendor_keyboard_input& report) const {
    return post_report(
        pqrs::karabiner::driverkit::virtual_hid_device::user_client_method::virtual_hid_keyboard_post_report,
        &report,
        sizeof(report));
  }

  kern_return_t post_report(const pqrs::karabiner::driverkit::virtual_hid_device::hid_report::apple_vendor_top_case_input& report) const {
    return post_report(
        pqrs::karabiner::driverkit::virtual_hid_device::user_client_method::virtual_hid_keyboard_post_report,
        &report,
        sizeof(report));
  }

  kern_return_t post_report(const pqrs::karabiner::driverkit::virtual_hid_device::hid_report::pointing_input& report) const {
    return post_report(
        pqrs::karabiner::driverkit::virtual_hid_device::user_client_method::virtual_hid_pointing_post_report,
        &report,
        sizeof(report));
  }

private:
  kern_return_t call(pqrs::karabiner::driverkit::virtual_hid_device::user_client_method user_client_method) const {
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

  kern_return_t call_scalar_method(pqrs::karabiner::driverkit::virtual_hid_device::user_client_method user_client_method,
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

  std::optional<bool> call_ready(pqrs::karabiner::driverkit::virtual_hid_device::user_client_method user_client_method) const {
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

    return static_cast<bool>(output[0]);
  }

  kern_return_t post_report(pqrs::karabiner::driverkit::virtual_hid_device::user_client_method user_client_method,
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
