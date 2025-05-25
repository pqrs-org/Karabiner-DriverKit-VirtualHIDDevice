#pragma once

#include "logger.hpp"
#include "version.hpp"
#include <IOKit/IOKitLib.h>
#include <array>
#include <gsl/gsl>
#include <nod/nod.hpp>
#include <optional>
#include <os/log.h>
#include <pqrs/dispatcher.hpp>
#include <pqrs/hid.hpp>
#include <pqrs/karabiner/driverkit/client_protocol_version.hpp>
#include <pqrs/karabiner/driverkit/driver_version.hpp>
#include <pqrs/karabiner/driverkit/virtual_hid_device_driver.hpp>
#include <pqrs/karabiner/driverkit/virtual_hid_device_service.hpp>
#include <pqrs/osx/iokit_return.hpp>
#include <pqrs/osx/iokit_service_monitor.hpp>

class io_service_client final : public pqrs::dispatcher::extra::dispatcher_client {
public:
  // Signals (invoked from the dispatcher thread)

  nod::signal<void(void)> opened;
  nod::signal<void(void)> closed;

  // Methods

  io_service_client(std::shared_ptr<pqrs::cf::run_loop_thread> run_loop_thread,
                    const std::string& virtual_hid_device_service_client_endpoint_filename)
      : dispatcher_client(),
        run_loop_thread_(run_loop_thread),
        virtual_hid_device_service_client_endpoint_filename_(virtual_hid_device_service_client_endpoint_filename),
        service_name_("org_pqrs_Karabiner_DriverKit_VirtualHIDDeviceRoot") {
    logger::get_logger()->info("{0} io_service_client::{1}",
                               virtual_hid_device_service_client_endpoint_filename_,
                               __func__);
  }

  ~io_service_client(void) {
    logger::get_logger()->info("{0} io_service_client::{1}",
                               virtual_hid_device_service_client_endpoint_filename_,
                               __func__);

    detach_from_dispatcher([this] {
      if (auto s = matched_services_.find_opened()) {
        close_connection(s->get_registry_entry_id());
      }

      service_monitor_ = nullptr;
    });
  }

  bool driver_activated(void) const {
    auto service = IOServiceGetMatchingService(type_safe::get(pqrs::osx::iokit_mach_port::null),
                                               IOServiceNameMatching(service_name_.c_str()));
    if (!service) {
      return false;
    }

    IOObjectRelease(service);
    return true;
  }

  bool driver_connected(void) const {
    std::lock_guard<std::mutex> lock(driver_version_mutex_);

    return driver_version_ != std::nullopt;
  }

  bool driver_version_mismatched(void) const {
    std::lock_guard<std::mutex> lock(driver_version_mutex_);

    // Return false until driver is loaded to avoid treating it as unmatched at startup.
    if (driver_version_ == std::nullopt) {
      return false;
    }

    if (driver_version_ == pqrs::karabiner::driverkit::driver_version::embedded_driver_version) {
      return false;
    } else {
      auto message = fmt::format("{0} driver_version_ is mismatched: Karabiner-VirtualHIDDevice-Daemon expected: {1}, actual dext: {2}",
                                 virtual_hid_device_service_client_endpoint_filename_,
                                 type_safe::get(pqrs::karabiner::driverkit::driver_version::embedded_driver_version),
                                 type_safe::get(*driver_version_));
      if (driver_version_mismatched_log_message_ != message) {
        driver_version_mismatched_log_message_ = message;
        logger::get_logger()->warn(message);
      }

      return true;
    }
  }

  std::optional<bool> get_virtual_hid_keyboard_ready(void) const {
    if (!driver_connected() ||
        driver_version_mismatched()) {
      return std::nullopt;
    }

    {
      std::lock_guard<std::mutex> lock(virtual_hid_keyboard_ready_mutex_);

      return virtual_hid_keyboard_ready_;
    }
  }

  std::optional<bool> get_virtual_hid_pointing_ready(void) const {
    if (!driver_connected() ||
        driver_version_mismatched()) {
      return std::nullopt;
    }

    {
      std::lock_guard<std::mutex> lock(virtual_hid_pointing_ready_mutex_);

      return virtual_hid_pointing_ready_;
    }
  }

  void async_start(void) {
    logger::get_logger()->info("{0} io_service_client::{1}",
                               virtual_hid_device_service_client_endpoint_filename_,
                               __func__);

    enqueue_to_dispatcher([this] {
      if (auto matching_dictionary = IOServiceNameMatching(service_name_.c_str())) {
        logger::get_logger()->info("{0} create service_monitor_",
                                   virtual_hid_device_service_client_endpoint_filename_);

        service_monitor_ = std::make_unique<pqrs::osx::iokit_service_monitor>(weak_dispatcher_,
                                                                              run_loop_thread_,
                                                                              matching_dictionary);

        service_monitor_->service_matched.connect([this](auto&& registry_entry_id, auto&& service_ptr) {
          logger::get_logger()->info("{0} iokit_service_monitor::service_matched",
                                     virtual_hid_device_service_client_endpoint_filename_);

          matched_services_.insert(registry_entry_id,
                                   service_ptr);

          logger::get_logger()->info("{0} matched_services_ size: {1}",
                                     virtual_hid_device_service_client_endpoint_filename_,
                                     matched_services_.get_services().size());

          open_connection();
        });

        service_monitor_->service_terminated.connect([this](auto&& registry_entry_id) {
          logger::get_logger()->info("{0} iokit_service_monitor::service_terminated",
                                     virtual_hid_device_service_client_endpoint_filename_);

          close_connection(registry_entry_id);

          matched_services_.erase(registry_entry_id);

          logger::get_logger()->info("{0} matched_services_ size: {1}",
                                     virtual_hid_device_service_client_endpoint_filename_,
                                     matched_services_.get_services().size());

          // If the alive connection is closed by `close_connection`,
          // we attempt to connect to the next available service.
          open_connection();
        });

        service_monitor_->error_occurred.connect([this](auto&& message, auto&& kern_return) {
          logger::get_logger()->error("{0} iokit_service_monitor {1} {2}",
                                      virtual_hid_device_service_client_endpoint_filename_,
                                      message,
                                      kern_return);
        });

        logger::get_logger()->info("{0} service_monitor_->async_start()",
                                   virtual_hid_device_service_client_endpoint_filename_);

        service_monitor_->async_start();

        CFRelease(matching_dictionary);
      }
    });
  }

  void async_virtual_hid_keyboard_initialize(const pqrs::karabiner::driverkit::virtual_hid_device_service::virtual_hid_keyboard_parameters& parameters) const {
    logger::get_logger()->info("{0} io_service_client::{1}",
                               virtual_hid_device_service_client_endpoint_filename_,
                               __func__);

    enqueue_to_dispatcher([this, parameters] {
      std::array<uint64_t, 3> input = {
          type_safe::get(parameters.get_vendor_id()),
          type_safe::get(parameters.get_product_id()),
          type_safe::get(parameters.get_country_code()),
      };

      auto r = call_scalar_method(pqrs::karabiner::driverkit::virtual_hid_device_driver::user_client_method::virtual_hid_keyboard_initialize,
                                  input.data(),
                                  input.size());

      if (!r) {
        logger::get_logger()->error("{0} virtual_hid_keyboard_initialize error: {1}",
                                    virtual_hid_device_service_client_endpoint_filename_,
                                    r.to_string());
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
        logger::get_logger()->error("{0} virtual_hid_keyboard_reset error: {1}",
                                    virtual_hid_device_service_client_endpoint_filename_,
                                    r.to_string());
      }
    });
  }

  void async_virtual_hid_pointing_initialize(void) const {
    logger::get_logger()->info("{0} io_service_client::{1}",
                               virtual_hid_device_service_client_endpoint_filename_,
                               __func__);

    enqueue_to_dispatcher([this] {
      auto r = call(pqrs::karabiner::driverkit::virtual_hid_device_driver::user_client_method::virtual_hid_pointing_initialize);

      if (!r) {
        logger::get_logger()->error("{0} virtual_hid_pointing_initialize error: {1}",
                                    virtual_hid_device_service_client_endpoint_filename_,
                                    r.to_string());
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
        logger::get_logger()->error("{0} virtual_hid_pointing_reset error: {1}",
                                    virtual_hid_device_service_client_endpoint_filename_,
                                    r.to_string());
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
        logger::get_logger()->error("{0} virtual_hid_keyboard_post_report(keyboard_input) error: {1}",
                                    virtual_hid_device_service_client_endpoint_filename_,
                                    r.to_string());
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
        logger::get_logger()->error("{0} virtual_hid_keyboard_post_report(consumer_input) error: {1}",
                                    virtual_hid_device_service_client_endpoint_filename_,
                                    r.to_string());
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
        logger::get_logger()->error("{0} virtual_hid_keyboard_post_report(apple_vendor_keyboard_input) error: {1}",
                                    virtual_hid_device_service_client_endpoint_filename_,
                                    r.to_string());
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
        logger::get_logger()->error("{0} virtual_hid_keyboard_post_report(apple_vendor_top_case_input) error: {1}",
                                    virtual_hid_device_service_client_endpoint_filename_,
                                    r.to_string());
      }
    });
  }

  void async_post_report(const pqrs::karabiner::driverkit::virtual_hid_device_driver::hid_report::generic_desktop_input& report) const {
    enqueue_to_dispatcher([this, report] {
      auto r = post_report(
          pqrs::karabiner::driverkit::virtual_hid_device_driver::user_client_method::virtual_hid_keyboard_post_report,
          &report,
          sizeof(report));

      if (!r) {
        logger::get_logger()->error("{0} virtual_hid_keyboard_post_report(generic_desktop_input) error: {1}",
                                    virtual_hid_device_service_client_endpoint_filename_,
                                    r.to_string());
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
        logger::get_logger()->error("{0} virtual_hid_pointing_post_report(pointing_input) error: {1}",
                                    virtual_hid_device_service_client_endpoint_filename_,
                                    r.to_string());
      }
    });
  }

private:
  class matched_service final {
  public:
    matched_service(pqrs::osx::iokit_registry_entry_id::value_t registry_entry_id,
                    pqrs::osx::iokit_object_ptr service)
        : registry_entry_id_(registry_entry_id),
          service_(service),
          opened_(false),
          invalidated_(false) {
    }

    pqrs::osx::iokit_registry_entry_id::value_t get_registry_entry_id(void) const {
      return registry_entry_id_;
    }

    pqrs::osx::iokit_object_ptr get_service(void) const {
      return service_;
    }

    bool get_opened(void) const {
      return opened_;
    }

    void set_opened(bool value) {
      opened_ = value;
    }

    bool get_invalidated(void) const {
      return invalidated_;
    }

    void set_invalidated(bool value) {
      invalidated_ = value;
    }

  private:
    pqrs::osx::iokit_registry_entry_id::value_t registry_entry_id_;
    pqrs::osx::iokit_object_ptr service_;
    bool opened_;
    // `invalidated_` is set to true if the driver version cannot be retrieved.
    bool invalidated_;
  };

  class matched_services final {
  public:
    void insert(pqrs::osx::iokit_registry_entry_id::value_t registry_entry_id,
                pqrs::osx::iokit_object_ptr service) {
      if (std::ranges::any_of(services_,
                              [registry_entry_id](const auto& s) {
                                return s->get_registry_entry_id() == registry_entry_id;
                              })) {
        return;
      }

      services_.push_back(std::make_shared<matched_service>(registry_entry_id, service));
    }

    void erase(pqrs::osx::iokit_registry_entry_id::value_t registry_entry_id) {
      std::erase_if(services_,
                    [registry_entry_id](const auto& s) {
                      return s->get_registry_entry_id() == registry_entry_id;
                    });
    }

    const std::vector<gsl::not_null<std::shared_ptr<matched_service>>>& get_services(void) const {
      return services_;
    }

    std::shared_ptr<matched_service> find(pqrs::osx::iokit_registry_entry_id::value_t registry_entry_id) const {
      auto it = std::ranges::find_if(services_,
                                     [registry_entry_id](const auto& s) {
                                       return s->get_registry_entry_id() == registry_entry_id;
                                     });
      if (it == std::end(services_)) {
        return nullptr;
      }

      return *it;
    }

    std::shared_ptr<matched_service> find_opened(void) const {
      auto it = std::ranges::find_if(services_,
                                     [](const auto& s) {
                                       return s->get_opened();
                                     });
      if (it == std::end(services_)) {
        return nullptr;
      }

      return *it;
    }

  private:
    std::vector<gsl::not_null<std::shared_ptr<matched_service>>> services_;
  };

  // This method is executed in the dispatcher thread.
  void set_driver_version(std::optional<pqrs::karabiner::driverkit::driver_version::value_t> value) {
    std::lock_guard<std::mutex> lock(driver_version_mutex_);

    if (driver_version_ != value) {
      driver_version_ = value;

      if (value) {
        logger::get_logger()->info(
            "{0} driver_version_ is changed: {1}",
            virtual_hid_device_service_client_endpoint_filename_,
            type_safe::get(*value));
      } else {
        logger::get_logger()->info(
            "{0} driver_version_ is changed: std::nullopt",
            virtual_hid_device_service_client_endpoint_filename_);
      }
    }
  }

  // This method is executed in the dispatcher thread.
  void set_virtual_hid_keyboard_ready(std::optional<bool> value) {
    std::lock_guard<std::mutex> lock(virtual_hid_keyboard_ready_mutex_);

    if (virtual_hid_keyboard_ready_ != value) {
      virtual_hid_keyboard_ready_ = value;

      logger::get_logger()->info(
          "{0} virtual_hid_keyboard_ready_ is changed: {1}",
          virtual_hid_device_service_client_endpoint_filename_,
          value ? (*value ? "true" : "false") : "std::nullopt");
    }
  }

  // This method is executed in the dispatcher thread.
  void set_virtual_hid_pointing_ready(std::optional<bool> value) {
    std::lock_guard<std::mutex> lock(virtual_hid_pointing_ready_mutex_);

    if (virtual_hid_pointing_ready_ != value) {
      virtual_hid_pointing_ready_ = value;

      logger::get_logger()->info(
          "{0} virtual_hid_pointing_ready_ is changed: {1}",
          virtual_hid_device_service_client_endpoint_filename_,
          value ? (*value ? "true" : "false") : "std::nullopt");
    }
  }

  // This method is executed in the dispatcher thread.
  void open_connection(void) {
    if (connection_) {
      return;
    }

    set_driver_version(std::nullopt);
    set_virtual_hid_keyboard_ready(std::nullopt);
    set_virtual_hid_pointing_ready(std::nullopt);

    for (const auto& matched_service : matched_services_.get_services()) {
      if (matched_service->get_invalidated()) {
        continue;
      }

      io_connect_t c;
      pqrs::osx::iokit_return r = IOServiceOpen(*(matched_service->get_service()),
                                                mach_task_self(),
                                                0,
                                                &c);

      if (!r) {
        logger::get_logger()->error("{0} io_service_client IOServiceOpen error: {1}",
                                    virtual_hid_device_service_client_endpoint_filename_,
                                    r.to_string());
        continue;
      }

      //
      // Check driver version
      //

      auto driver_version = call_driver_version(c);
      set_driver_version(driver_version);
      if (!driver_version) {
        logger::get_logger()->error("{0} io_service_client failed to get driver_version",
                                    virtual_hid_device_service_client_endpoint_filename_);
        IOServiceClose(c);
        matched_service->set_invalidated(true);
        continue;
      }

      connection_ = pqrs::osx::iokit_object_ptr(c);
      matched_service->set_opened(true);

      enqueue_to_dispatcher([this] {
        logger::get_logger()->info("{0} io_service_client::opened",
                                   virtual_hid_device_service_client_endpoint_filename_);

        opened();
      });

      break;
    }
  }

  // This method is executed in the dispatcher thread.
  void close_connection(pqrs::osx::iokit_registry_entry_id::value_t registry_entry_id) {
    // If no connection is open for the given registry_entry_id, do nothing.
    auto matched_service = matched_services_.find(registry_entry_id);
    if (!matched_service ||
        !matched_service->get_opened()) {
      return;
    }
    matched_service->set_opened(false);

    if (!connection_) {
      return;
    }

    IOServiceClose(*connection_);
    connection_.reset();

    enqueue_to_dispatcher([this] {
      logger::get_logger()->info("{0} io_service_client::closed",
                                 virtual_hid_device_service_client_endpoint_filename_);

      closed();
    });

    set_driver_version(std::nullopt);
    set_virtual_hid_keyboard_ready(std::nullopt);
    set_virtual_hid_pointing_ready(std::nullopt);
  }

  // This method is executed in the dispatcher thread.
  std::optional<pqrs::karabiner::driverkit::driver_version::value_t> call_driver_version(io_connect_t connection) const {
    if (!connection) {
      return std::nullopt;
    }

    // Do not call `driver_connected()` here.
    // Do not call `driver_version_mismatched()` here.

    uint64_t output[1] = {0};
    uint32_t output_count = 1;
    auto kr = IOConnectCallScalarMethod(connection,
                                        static_cast<uint32_t>(pqrs::karabiner::driverkit::virtual_hid_device_driver::user_client_method::driver_version),
                                        nullptr,
                                        0,
                                        output,
                                        &output_count);

    if (kr != kIOReturnSuccess) {
      return std::nullopt;
    }

    return pqrs::karabiner::driverkit::driver_version::value_t(output[0]);
  }

  // This method is executed in the dispatcher thread.
  pqrs::osx::iokit_return call(pqrs::karabiner::driverkit::virtual_hid_device_driver::user_client_method user_client_method) const {
    if (!connection_) {
      return kIOReturnNotOpen;
    }

    if (!driver_connected() ||
        driver_version_mismatched()) {
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

    if (!driver_connected() ||
        driver_version_mismatched()) {
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

    if (!driver_connected() ||
        driver_version_mismatched()) {
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

    if (!driver_connected() ||
        driver_version_mismatched()) {
      return kIOReturnError;
    }

    return IOConnectCallStructMethod(*connection_,
                                     static_cast<uint32_t>(user_client_method),
                                     report,
                                     report_size,
                                     nullptr,
                                     0);
  }

  std::shared_ptr<pqrs::cf::run_loop_thread> run_loop_thread_;
  std::string virtual_hid_device_service_client_endpoint_filename_;
  std::string service_name_;
  std::unique_ptr<pqrs::osx::iokit_service_monitor> service_monitor_;
  matched_services matched_services_;
  pqrs::osx::iokit_object_ptr connection_;

  mutable std::mutex driver_version_mutex_;
  std::optional<pqrs::karabiner::driverkit::driver_version::value_t> driver_version_;
  // Remember last log message in order to suppress duplicated messages.
  mutable std::string driver_version_mismatched_log_message_;

  mutable std::mutex virtual_hid_keyboard_ready_mutex_;
  std::optional<bool> virtual_hid_keyboard_ready_;

  mutable std::mutex virtual_hid_pointing_ready_mutex_;
  std::optional<bool> virtual_hid_pointing_ready_;
};
