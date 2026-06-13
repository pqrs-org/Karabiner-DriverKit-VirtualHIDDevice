#pragma once

// (C) Copyright Takayama Fumihiko 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See https://www.boost.org/LICENSE_1_0.txt)

#include <IOKit/IOKitLib.h>
#include <pqrs/osx/iokit_types.hpp>

namespace pqrs::karabiner::driverkit::virtual_hid_device_service::utility {
inline bool driver_running() {
  auto service = IOServiceGetMatchingService(type_safe::get(pqrs::osx::iokit_mach_port::null),
                                             IOServiceNameMatching("org_pqrs_Karabiner_DriverKit_VirtualHIDDeviceRoot"));
  if (!service) {
    return false;
  }

  IOObjectRelease(service);
  return true;
}

inline bool virtual_hid_keyboard_exists() {
  auto service = IOServiceGetMatchingService(type_safe::get(pqrs::osx::iokit_mach_port::null),
                                             IOServiceNameMatching("org_pqrs_Karabiner_DriverKit_VirtualHIDKeyboard"));
  if (!service) {
    return false;
  }

  IOObjectRelease(service);
  return true;
}

inline bool virtual_hid_pointing_exists() {
  auto service = IOServiceGetMatchingService(type_safe::get(pqrs::osx::iokit_mach_port::null),
                                             IOServiceNameMatching("org_pqrs_Karabiner_DriverKit_VirtualHIDPointing"));
  if (!service) {
    return false;
  }

  IOObjectRelease(service);
  return true;
}
} // namespace pqrs::karabiner::driverkit::virtual_hid_device_service::utility
