#include <catch2/catch.hpp>

#include <pqrs/karabiner/driverkit/virtual_hid_device_driver.hpp>

TEST_CASE("sizeof") {
  using namespace pqrs::karabiner::driverkit::virtual_hid_device_driver;

  REQUIRE(sizeof(hid_report::apple_vendor_keyboard_input) == 65);
  REQUIRE(sizeof(hid_report::apple_vendor_top_case_input) == 65);
  REQUIRE(sizeof(hid_report::consumer_input) == 65);
  REQUIRE(sizeof(hid_report::keyboard_input) == 67);
  REQUIRE(sizeof(hid_report::pointing_input) == 8);
}
