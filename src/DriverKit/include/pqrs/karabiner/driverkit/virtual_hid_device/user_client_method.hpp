#pragma once

namespace pqrs {
namespace karabiner {
namespace driverkit {
namespace virtual_hid_device {

enum class user_client_method {
  //
  // keyboard
  //

  post_keyboard_input_report,
  post_consumer_input_report,
  post_apple_vendor_keyboard_input_report,
  post_apple_vendor_top_case_input_report,
  reset_virtual_hid_keyboard,

  //
  // pointing
  //

  post_pointing_input_report,
  reset_virtual_hid_pointing,

  end_,
};

} // namespace virtual_hid_device
} // namespace driverkit
} // namespace karabiner
} // namespace pqrs
