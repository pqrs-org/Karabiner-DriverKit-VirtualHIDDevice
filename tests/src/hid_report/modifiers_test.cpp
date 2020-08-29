#include <catch2/catch.hpp>

#include <pqrs/karabiner/driverkit/virtual_hid_device_driver.hpp>

TEST_CASE("modifiers") {
  using namespace pqrs::karabiner::driverkit::virtual_hid_device_driver;

  {
    hid_report::modifiers modifiers;
    REQUIRE(modifiers.get_raw_value() == 0x0);
    REQUIRE(modifiers.empty());
    REQUIRE(!modifiers.exists(hid_report::modifier::left_control));
    REQUIRE(!modifiers.exists(hid_report::modifier::left_shift));
    REQUIRE(!modifiers.exists(hid_report::modifier::left_option));
    REQUIRE(!modifiers.exists(hid_report::modifier::left_command));
    REQUIRE(!modifiers.exists(hid_report::modifier::right_control));
    REQUIRE(!modifiers.exists(hid_report::modifier::right_shift));
    REQUIRE(!modifiers.exists(hid_report::modifier::right_option));
    REQUIRE(!modifiers.exists(hid_report::modifier::right_command));

    modifiers.insert(hid_report::modifier::left_control);
    REQUIRE(modifiers.get_raw_value() == 0x1);
    REQUIRE(!modifiers.empty());
    REQUIRE(modifiers.exists(hid_report::modifier::left_control));
    REQUIRE(!modifiers.exists(hid_report::modifier::left_shift));
    REQUIRE(!modifiers.exists(hid_report::modifier::left_option));
    REQUIRE(!modifiers.exists(hid_report::modifier::left_command));
    REQUIRE(!modifiers.exists(hid_report::modifier::right_control));
    REQUIRE(!modifiers.exists(hid_report::modifier::right_shift));
    REQUIRE(!modifiers.exists(hid_report::modifier::right_option));
    REQUIRE(!modifiers.exists(hid_report::modifier::right_command));

    modifiers.insert(hid_report::modifier::right_control);
    REQUIRE(modifiers.get_raw_value() == 0x11);
    REQUIRE(!modifiers.empty());
    REQUIRE(modifiers.exists(hid_report::modifier::left_control));
    REQUIRE(!modifiers.exists(hid_report::modifier::left_shift));
    REQUIRE(!modifiers.exists(hid_report::modifier::left_option));
    REQUIRE(!modifiers.exists(hid_report::modifier::left_command));
    REQUIRE(modifiers.exists(hid_report::modifier::right_control));
    REQUIRE(!modifiers.exists(hid_report::modifier::right_shift));
    REQUIRE(!modifiers.exists(hid_report::modifier::right_option));
    REQUIRE(!modifiers.exists(hid_report::modifier::right_command));

    modifiers.erase(hid_report::modifier::left_shift);
    REQUIRE(modifiers.get_raw_value() == 0x11);
    REQUIRE(!modifiers.empty());
    REQUIRE(modifiers.exists(hid_report::modifier::left_control));
    REQUIRE(!modifiers.exists(hid_report::modifier::left_shift));
    REQUIRE(!modifiers.exists(hid_report::modifier::left_option));
    REQUIRE(!modifiers.exists(hid_report::modifier::left_command));
    REQUIRE(modifiers.exists(hid_report::modifier::right_control));
    REQUIRE(!modifiers.exists(hid_report::modifier::right_shift));
    REQUIRE(!modifiers.exists(hid_report::modifier::right_option));
    REQUIRE(!modifiers.exists(hid_report::modifier::right_command));

    modifiers.erase(hid_report::modifier::left_control);
    REQUIRE(modifiers.get_raw_value() == 0x10);
    REQUIRE(!modifiers.empty());
    REQUIRE(!modifiers.exists(hid_report::modifier::left_control));
    REQUIRE(!modifiers.exists(hid_report::modifier::left_shift));
    REQUIRE(!modifiers.exists(hid_report::modifier::left_option));
    REQUIRE(!modifiers.exists(hid_report::modifier::left_command));
    REQUIRE(modifiers.exists(hid_report::modifier::right_control));
    REQUIRE(!modifiers.exists(hid_report::modifier::right_shift));
    REQUIRE(!modifiers.exists(hid_report::modifier::right_option));
    REQUIRE(!modifiers.exists(hid_report::modifier::right_command));

    modifiers.clear();
    REQUIRE(modifiers.get_raw_value() == 0x0);
    REQUIRE(modifiers.empty());
    REQUIRE(!modifiers.exists(hid_report::modifier::left_control));
    REQUIRE(!modifiers.exists(hid_report::modifier::left_shift));
    REQUIRE(!modifiers.exists(hid_report::modifier::left_option));
    REQUIRE(!modifiers.exists(hid_report::modifier::left_command));
    REQUIRE(!modifiers.exists(hid_report::modifier::right_control));
    REQUIRE(!modifiers.exists(hid_report::modifier::right_shift));
    REQUIRE(!modifiers.exists(hid_report::modifier::right_option));
    REQUIRE(!modifiers.exists(hid_report::modifier::right_command));
  }
}
