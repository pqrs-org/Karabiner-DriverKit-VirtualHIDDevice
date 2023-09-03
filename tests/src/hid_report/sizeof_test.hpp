#include <boost/ut.hpp>
#include <pqrs/karabiner/driverkit/virtual_hid_device_driver.hpp>

void run_sizeof_test(void) {
  using namespace boost::ut;
  using namespace boost::ut::literals;

  "sizeof"_test = [] {
    using namespace pqrs::karabiner::driverkit::virtual_hid_device_driver;

    expect(sizeof(hid_report::apple_vendor_keyboard_input) == 65_ul);
    expect(sizeof(hid_report::apple_vendor_top_case_input) == 65_ul);
    expect(sizeof(hid_report::consumer_input) == 65_ul);
    expect(sizeof(hid_report::generic_desktop_input) == 65_ul);
    expect(sizeof(hid_report::keyboard_input) == 67_ul);
    expect(sizeof(hid_report::pointing_input) == 8_ul);
  };
}
