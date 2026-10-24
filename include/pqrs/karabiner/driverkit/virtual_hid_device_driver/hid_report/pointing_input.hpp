#pragma once

// (C) Copyright Takayama Fumihiko 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See https://www.boost.org/LICENSE_1_0.txt)

#include "buttons.hpp"
#include <cstdint>
#include <cstring>

namespace pqrs::karabiner::driverkit::virtual_hid_device_driver::hid_report {

class __attribute__((packed)) pointing_input final {
public:
  pointing_input() = default;
  bool operator==(const pointing_input& other) const { return (memcmp(this, &other, sizeof(*this)) == 0); }
  bool operator!=(const pointing_input& other) const { return !(*this == other); }

  buttons buttons{};
  uint8_t x{0};
  uint8_t y{0};
  uint8_t vertical_wheel{0};
  uint8_t horizontal_wheel{0};
};
} // namespace pqrs::karabiner::driverkit::virtual_hid_device_driver::hid_report
