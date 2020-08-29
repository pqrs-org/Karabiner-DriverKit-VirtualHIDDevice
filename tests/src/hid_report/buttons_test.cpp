#include <catch2/catch.hpp>

#include <pqrs/karabiner/driverkit/virtual_hid_device_driver.hpp>

TEST_CASE("buttons") {
  using namespace pqrs::karabiner::driverkit::virtual_hid_device_driver;

  {
    hid_report::buttons buttons;
    REQUIRE(buttons.get_raw_value() == 0);
    REQUIRE(buttons.empty());

    buttons.insert(1);
    REQUIRE(buttons.get_raw_value() == 0x1);
    REQUIRE(!buttons.empty());

    buttons.insert(32);
    REQUIRE(buttons.get_raw_value() == 0x80000001);
    REQUIRE(!buttons.empty());

    buttons.insert(0);
    REQUIRE(buttons.get_raw_value() == 0x80000001);
    REQUIRE(!buttons.empty());

    buttons.insert(33);
    REQUIRE(buttons.get_raw_value() == 0x80000001);
    REQUIRE(!buttons.empty());

    buttons.erase(1);
    REQUIRE(buttons.get_raw_value() == 0x80000000);
    REQUIRE(!buttons.empty());

    buttons.clear();
    REQUIRE(buttons.get_raw_value() == 0);
    REQUIRE(buttons.empty());
  }
}
