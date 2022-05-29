#include <boost/ut.hpp>
#include <pqrs/karabiner/driverkit/virtual_hid_device_driver.hpp>

void run_buttons_test(void) {
  using namespace boost::ut;
  using namespace boost::ut::literals;

  "buttons"_test = [] {
    using namespace pqrs::karabiner::driverkit::virtual_hid_device_driver;

    {
      hid_report::buttons buttons;
      expect(buttons.get_raw_value() == 0);
      expect(buttons.empty());

      buttons.insert(1);
      expect(buttons.get_raw_value() == 0x1);
      expect(!buttons.empty());

      buttons.insert(32);
      expect(buttons.get_raw_value() == 0x80000001);
      expect(!buttons.empty());

      buttons.insert(0);
      expect(buttons.get_raw_value() == 0x80000001);
      expect(!buttons.empty());

      buttons.insert(33);
      expect(buttons.get_raw_value() == 0x80000001);
      expect(!buttons.empty());

      buttons.erase(1);
      expect(buttons.get_raw_value() == 0x80000000);
      expect(!buttons.empty());

      buttons.clear();
      expect(buttons.get_raw_value() == 0);
      expect(buttons.empty());
    }
  };
}
