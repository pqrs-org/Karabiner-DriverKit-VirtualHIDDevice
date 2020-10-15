#pragma once

#include "logger.hpp"
#include "version.hpp"
#include <IOKit/IOKitLib.h>
#include <array>
#include <nod/nod.hpp>
#include <optional>
#include <os/log.h>
#include <pqrs/dispatcher.hpp>
#include <pqrs/hid.hpp>
#include <pqrs/karabiner/driverkit/virtual_hid_device_driver.hpp>
#include <pqrs/osx/iokit_return.hpp>
#include <pqrs/osx/iokit_service_monitor.hpp>

class io_service_client final : public pqrs::dispatcher::extra::dispatcher_client {
public:
  // Signals (invoked from the dispatcher thread)

  nod::signal<void(void)> opened;
  nod::signal<void(void)> closed;

  // Methods

  io_service_client(void) : dispatcher_client() {
  }

  ~io_service_client(void) {
    detach_from_dispatcher([this] {
      close_connection();

      service_monitor_ = nullptr;
    });
  }

  bool driver_loaded(void) const {
    std::lock_guard<std::mutex> lock(driver_version_mutex_);

    return driver_version_ != std::nullopt;
  }

  bool driver_version_matched(void) const {
    std::lock_guard<std::mutex> lock(driver_version_mutex_);

    if (driver_version_ == DRIVER_VERSION_NUMBER) {
      return true;
    } else {
      if (driver_version_) {
        logger::get_logger()->warn("driver_version_ is mismatched: client expected: {0}, actual dext: {1}",
                                   DRIVER_VERSION_NUMBER,
                                   *driver_version_);
      } else {
        logger::get_logger()->warn("driver_version_ is mismatched: client expected: {0}, actual dext: std::nullopt",
                                   DRIVER_VERSION_NUMBER);
      }

      return false;
    }
  }

  std::optional<bool> get_virtual_hid_keyboard_ready(void) const {
    std::lock_guard<std::mutex> lock(virtual_hid_keyboard_ready_mutex_);

    return virtual_hid_keyboard_ready_;
  }

  std::optional<bool> get_virtual_hid_pointing_ready(void) const {
    std::lock_guard<std::mutex> lock(virtual_hid_pointing_ready_mutex_);

    return virtual_hid_pointing_ready_;
  }

  void async_start(void) {
    logger::get_logger()->info("io_service_client::{0}", __func__);

    enqueue_to_dispatcher([this] {
      if (auto matching_dictionary = IOServiceNameMatching("org_pqrs_Karabiner_DriverKit_VirtualHIDDeviceRoot")) {
        service_monitor_ = std::make_unique<pqrs::osx::iokit_service_monitor>(weak_dispatcher_,
                                                                              matching_dictionary);

        service_monitor_->service_matched.connect([this](auto&& registry_entry_id, auto&& service_ptr) {
          close_connection();

          // Use the last matched service.
          open_connection(service_ptr);
        });

        service_monitor_->service_terminated.connect([this](auto&& registry_entry_id) {
          close_connection();

          // Use the next service
          service_monitor_->async_invoke_service_matched();
        });

        service_monitor_->async_start();

        CFRelease(matching_dictionary);
      }
    });
  }

  void async_virtual_hid_keyboard_initialize(pqrs::hid::country_code::value_t country_code) const {
    logger::get_logger()->info("io_service_client::{0}", __func__);

    enqueue_to_dispatcher([this, country_code] {
      std::array<uint64_t, 1> input = {type_safe::get(country_code)};

      auto r = call_scalar_method(pqrs::karabiner::driverkit::virtual_hid_device_driver::user_client_method::virtual_hid_keyboard_initialize,
                                  input.data(),
                                  input.size());

      if (!r) {
        logger::get_logger()->error("virtual_hid_keyboard_initialize error: {0}", r.to_string());
      }
    });
  }

  void async_virtual_hid_keyboard_ready(void) {
    enqueue_to_dispatcher([this] {
      auto ready = call_ready(pqrs::karabiner::driverkit::virtual_hid_device_driver::user_client_method::virtual_hid_keyboard_ready);

      enqueue_to_dispatcher([this, ready] {
        set_virtual_hid_keyboard_ready(ready);
      });
    });
  }

  void async_virtual_hid_keyboard_reset(void) const {
    enqueue_to_dispatcher([this] {
      auto r = call(pqrs::karabiner::driverkit::virtual_hid_device_driver::user_client_method::virtual_hid_keyboard_reset);

      if (!r) {
        logger::get_logger()->error("virtual_hid_keyboard_reset error: {0}", r.to_string());
      }
    });
  }

  void async_virtual_hid_pointing_initialize(void) const {
    logger::get_logger()->info("io_service_client::{0}", __func__);

    enqueue_to_dispatcher([this] {
      auto r = call(pqrs::karabiner::driverkit::virtual_hid_device_driver::user_client_method::virtual_hid_pointing_initialize);

      if (!r) {
        logger::get_logger()->error("virtual_hid_pointing_initialize error: {0}", r.to_string());
      }
    });
  }

  void async_virtual_hid_pointing_ready(void) {
    enqueue_to_dispatcher([this] {
      auto ready = call_ready(pqrs::karabiner::driverkit::virtual_hid_device_driver::user_client_method::virtual_hid_pointing_ready);

      enqueue_to_dispatcher([this, ready] {
        set_virtual_hid_pointing_ready(ready);
      });
    });
  }

  void async_virtual_hid_pointing_reset(void) const {
    enqueue_to_dispatcher([this] {
      auto r = call(pqrs::karabiner::driverkit::virtual_hid_device_driver::user_client_method::virtual_hid_pointing_reset);

      if (!r) {
        logger::get_logger()->error("virtual_hid_pointing_reset error: {0}", r.to_string());
      }
    });
  }

  void async_post_report(const pqrs::karabiner::driverkit::virtual_hid_device_driver::hid_report::keyboard_input& report) const {
    enqueue_to_dispatcher([this, report] {
      auto r = post_report(
          pqrs::karabiner::driverkit::virtual_hid_device_driver::user_client_method::virtual_hid_keyboard_post_report,
          &report,
          sizeof(report));

      if (!r) {
        logger::get_logger()->error("virtual_hid_keyboard_post_report(keyboard_input) error: {0}", r.to_string());
      }
    });
  }

  void async_post_report(const pqrs::karabiner::driverkit::virtual_hid_device_driver::hid_report::consumer_input& report) const {
    enqueue_to_dispatcher([this, report] {
      auto r = post_report(
          pqrs::karabiner::driverkit::virtual_hid_device_driver::user_client_method::virtual_hid_keyboard_post_report,
          &report,
          sizeof(report));

      if (!r) {
        logger::get_logger()->error("virtual_hid_keyboard_post_report(consumer_input) error: {0}", r.to_string());
      }
    });
  }

  void async_post_report(const pqrs::karabiner::driverkit::virtual_hid_device_driver::hid_report::apple_vendor_keyboard_input& report) const {
    enqueue_to_dispatcher([this, report] {
      auto r = post_report(
          pqrs::karabiner::driverkit::virtual_hid_device_driver::user_client_method::virtual_hid_keyboard_post_report,
          &report,
          sizeof(report));

      if (!r) {
        logger::get_logger()->error("virtual_hid_keyboard_post_report(apple_vendor_keyboard_input) error: {0}", r.to_string());
      }
    });
  }

  void async_post_report(const pqrs::karabiner::driverkit::virtual_hid_device_driver::hid_report::apple_vendor_top_case_input& report) const {
    enqueue_to_dispatcher([this, report] {
      auto r = post_report(
          pqrs::karabiner::driverkit::virtual_hid_device_driver::user_client_method::virtual_hid_keyboard_post_report,
          &report,
          sizeof(report));

      if (!r) {
        logger::get_logger()->error("virtual_hid_keyboard_post_report(apple_vendor_top_case_input) error: {0}", r.to_string());
      }
    });
  }

  void async_post_report(const pqrs::karabiner::driverkit::virtual_hid_device_driver::hid_report::pointing_input& report) const {
    enqueue_to_dispatcher([this, report] {
      auto r = post_report(
          pqrs::karabiner::driverkit::virtual_hid_device_driver::user_client_method::virtual_hid_pointing_post_report,
          &report,
          sizeof(report));

      if (!r) {
        logger::get_logger()->error("virtual_hid_pointing_post_report(pointing_input) error: {0}", r.to_string());
      }
    });
  }

private:
  // This method is executed in the dispatcher thread.
  void set_driver_version(std::optional<uint64_t> value) {
    std::lock_guard<std::mutex> lock(driver_version_mutex_);

    if (driver_version_ != value) {
      driver_version_ = value;

      if (value) {
        logger::get_logger()->info(
            "driver_version_ is changed: {0}",
            *value);
      } else {
        logger::get_logger()->info(
            "driver_version_ is changed: std::nullopt");
      }
    }
  }

  // This method is executed in the dispatcher thread.
  void set_virtual_hid_keyboard_ready(std::optional<bool> value) {
    std::lock_guard<std::mutex> lock(virtual_hid_keyboard_ready_mutex_);

    if (virtual_hid_keyboard_ready_ != value) {
      virtual_hid_keyboard_ready_ = value;

      logger::get_logger()->info(
          "virtual_hid_keyboard_ready_ is changed: {0}",
          value ? (*value ? "true" : "false") : "std::nullopt");
    }
  }

  // This method is executed in the dispatcher thread.
  void set_virtual_hid_pointing_ready(std::optional<bool> value) {
    std::lock_guard<std::mutex> lock(virtual_hid_pointing_ready_mutex_);

    if (virtual_hid_pointing_ready_ != value) {
      virtual_hid_pointing_ready_ = value;

      logger::get_logger()->info(
          "virtual_hid_pointing_ready_ is changed: {0}",
          value ? (*value ? "true" : "false") : "std::nullopt");
    }
  }

  // This method is executed in the dispatcher thread.
  void open_connection(pqrs::osx::iokit_object_ptr s) {
    if (connection_) {
      return;
    }

    set_driver_version(std::nullopt);
    set_virtual_hid_keyboard_ready(std::nullopt);
    set_virtual_hid_pointing_ready(std::nullopt);

    service_ = s;

    io_connect_t c;
    pqrs::osx::iokit_return r = IOServiceOpen(*service_, mach_task_self(), 0, &c);

    if (!r) {
      logger::get_logger()->error("io_service_client IOServiceOpen error: {0}", r.to_string());
      connection_.reset();
      return;
    }

    connection_ = pqrs::osx::iokit_object_ptr(c);

    //
    // Check driver version
    //

    auto driver_version = call_driver_version();
    set_driver_version(driver_version);
    if (!driver_version) {
      logger::get_logger()->error("io_service_client failed to get driver_version");
      connection_.reset();
      return;
    }

    enqueue_to_dispatcher([this] {
      logger::get_logger()->info("io_service_client::opened");

      opened();
    });
  }

  // This method is executed in the dispatcher thread.
  void close_connection(void) {
    if (connection_) {
      IOServiceClose(*connection_);
      connection_.reset();

      enqueue_to_dispatcher([this] {
        logger::get_logger()->info("io_service_client::closed");

        closed();
      });
    }

    service_.reset();

    set_driver_version(std::nullopt);
    set_virtual_hid_keyboard_ready(std::nullopt);
    set_virtual_hid_pointing_ready(std::nullopt);
  }

  // This method is executed in the dispatcher thread.
  std::optional<uint64_t> call_driver_version(void) const {
    if (!connection_) {
      return std::nullopt;
    }

    // Do not call `driver_version_matched()` here.

    uint64_t output[1] = {0};
    uint32_t output_count = 1;
    auto kr = IOConnectCallScalarMethod(*connection_,
                                        static_cast<uint32_t>(pqrs::karabiner::driverkit::virtual_hid_device_driver::user_client_method::driver_version),
                                        nullptr,
                                        0,
                                        output,
                                        &output_count);

    if (kr != kIOReturnSuccess) {
      return std::nullopt;
    }

    return output[0];
  }

  // This method is executed in the dispatcher thread.
  pqrs::osx::iokit_return call(pqrs::karabiner::driverkit::virtual_hid_device_driver::user_client_method user_client_method) const {
    if (!connection_) {
      return kIOReturnNotOpen;
    }

    if (!driver_version_matched()) {
      return kIOReturnError;
    }

    return IOConnectCallStructMethod(*connection_,
                                     static_cast<uint32_t>(user_client_method),
                                     nullptr,
                                     0,
                                     nullptr,
                                     0);
  }

  // This method is executed in the dispatcher thread.
  pqrs::osx::iokit_return call_scalar_method(pqrs::karabiner::driverkit::virtual_hid_device_driver::user_client_method user_client_method,
                                             const uint64_t* input,
                                             uint32_t input_count) const {
    if (!connection_) {
      return kIOReturnNotOpen;
    }

    if (!driver_version_matched()) {
      return kIOReturnError;
    }

    return IOConnectCallScalarMethod(*connection_,
                                     static_cast<uint32_t>(user_client_method),
                                     input,
                                     input_count,
                                     nullptr,
                                     0);
  }

  // This method is executed in the dispatcher thread.
  std::optional<bool> call_ready(pqrs::karabiner::driverkit::virtual_hid_device_driver::user_client_method user_client_method) const {
    if (!connection_) {
      return std::nullopt;
    }

    if (!driver_version_matched()) {
      return std::nullopt;
    }

    uint64_t output[1] = {0};
    uint32_t output_count = 1;
    auto kr = IOConnectCallScalarMethod(*connection_,
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

  // This method is executed in the dispatcher thread.
  pqrs::osx::iokit_return post_report(pqrs::karabiner::driverkit::virtual_hid_device_driver::user_client_method user_client_method,
                                      const void* report,
                                      size_t report_size) const {
    if (!connection_) {
      return kIOReturnNotOpen;
    }

    if (!driver_version_matched()) {
      return kIOReturnError;
    }

    return IOConnectCallStructMethod(*connection_,
                                     static_cast<uint32_t>(user_client_method),
                                     report,
                                     report_size,
                                     nullptr,
                                     0);
  }

  std::unique_ptr<pqrs::osx::iokit_service_monitor> service_monitor_;
  pqrs::osx::iokit_object_ptr service_;
  pqrs::osx::iokit_object_ptr connection_;

  mutable std::mutex driver_version_mutex_;
  std::optional<uint64_t> driver_version_;

  mutable std::mutex virtual_hid_keyboard_ready_mutex_;
  std::optional<bool> virtual_hid_keyboard_ready_;

  mutable std::mutex virtual_hid_pointing_ready_mutex_;
  std::optional<bool> virtual_hid_pointing_ready_;
};
