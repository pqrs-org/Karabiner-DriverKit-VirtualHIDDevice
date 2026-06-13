#pragma once

// (C) Copyright Takayama Fumihiko 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See https://www.boost.org/LICENSE_1_0.txt)

#include "keys.hpp"
#include "modifiers.hpp"
#include <cstdint>

namespace pqrs::karabiner::driverkit::virtual_hid_device_driver::hid_report {

class __attribute__((packed)) keyboard_input final {
public:
  keyboard_input() : report_id_(1), reserved(0) {}
  bool operator==(const keyboard_input& other) const { return (memcmp(this, &other, sizeof(*this)) == 0); }
  bool operator!=(const keyboard_input& other) const { return !(*this == other); }

private:
  uint8_t report_id_ __attribute__((unused));

public:
  modifiers modifiers;

private:
  uint8_t reserved __attribute__((unused));

public:
  keys keys;
};
} // namespace pqrs::karabiner::driverkit::virtual_hid_device_driver::hid_report
