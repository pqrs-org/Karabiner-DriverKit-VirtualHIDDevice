#include <boost/ut.hpp>
#include <pqrs/karabiner/driverkit/virtual_hid_device_driver.hpp>

void run_modifiers_test(void) {
  using namespace boost::ut;
  using namespace boost::ut::literals;

  "modifiers"_test = [] {
    using namespace pqrs::karabiner::driverkit::virtual_hid_device_driver;

    {
      hid_report::modifiers modifiers;
      expect(modifiers.get_raw_value() == 0x0);
      expect(modifiers.empty());
      expect(!modifiers.exists(hid_report::modifier::left_control));
      expect(!modifiers.exists(hid_report::modifier::left_shift));
      expect(!modifiers.exists(hid_report::modifier::left_option));
      expect(!modifiers.exists(hid_report::modifier::left_command));
      expect(!modifiers.exists(hid_report::modifier::right_control));
      expect(!modifiers.exists(hid_report::modifier::right_shift));
      expect(!modifiers.exists(hid_report::modifier::right_option));
      expect(!modifiers.exists(hid_report::modifier::right_command));

      modifiers.insert(hid_report::modifier::left_control);
      expect(modifiers.get_raw_value() == 0x1);
      expect(!modifiers.empty());
      expect(modifiers.exists(hid_report::modifier::left_control));
      expect(!modifiers.exists(hid_report::modifier::left_shift));
      expect(!modifiers.exists(hid_report::modifier::left_option));
      expect(!modifiers.exists(hid_report::modifier::left_command));
      expect(!modifiers.exists(hid_report::modifier::right_control));
      expect(!modifiers.exists(hid_report::modifier::right_shift));
      expect(!modifiers.exists(hid_report::modifier::right_option));
      expect(!modifiers.exists(hid_report::modifier::right_command));

      modifiers.insert(hid_report::modifier::right_control);
      expect(modifiers.get_raw_value() == 0x11);
      expect(!modifiers.empty());
      expect(modifiers.exists(hid_report::modifier::left_control));
      expect(!modifiers.exists(hid_report::modifier::left_shift));
      expect(!modifiers.exists(hid_report::modifier::left_option));
      expect(!modifiers.exists(hid_report::modifier::left_command));
      expect(modifiers.exists(hid_report::modifier::right_control));
      expect(!modifiers.exists(hid_report::modifier::right_shift));
      expect(!modifiers.exists(hid_report::modifier::right_option));
      expect(!modifiers.exists(hid_report::modifier::right_command));

      modifiers.erase(hid_report::modifier::left_shift);
      expect(modifiers.get_raw_value() == 0x11);
      expect(!modifiers.empty());
      expect(modifiers.exists(hid_report::modifier::left_control));
      expect(!modifiers.exists(hid_report::modifier::left_shift));
      expect(!modifiers.exists(hid_report::modifier::left_option));
      expect(!modifiers.exists(hid_report::modifier::left_command));
      expect(modifiers.exists(hid_report::modifier::right_control));
      expect(!modifiers.exists(hid_report::modifier::right_shift));
      expect(!modifiers.exists(hid_report::modifier::right_option));
      expect(!modifiers.exists(hid_report::modifier::right_command));

      modifiers.erase(hid_report::modifier::left_control);
      expect(modifiers.get_raw_value() == 0x10);
      expect(!modifiers.empty());
      expect(!modifiers.exists(hid_report::modifier::left_control));
      expect(!modifiers.exists(hid_report::modifier::left_shift));
      expect(!modifiers.exists(hid_report::modifier::left_option));
      expect(!modifiers.exists(hid_report::modifier::left_command));
      expect(modifiers.exists(hid_report::modifier::right_control));
      expect(!modifiers.exists(hid_report::modifier::right_shift));
      expect(!modifiers.exists(hid_report::modifier::right_option));
      expect(!modifiers.exists(hid_report::modifier::right_command));

      modifiers.clear();
      expect(modifiers.get_raw_value() == 0x0);
      expect(modifiers.empty());
      expect(!modifiers.exists(hid_report::modifier::left_control));
      expect(!modifiers.exists(hid_report::modifier::left_shift));
      expect(!modifiers.exists(hid_report::modifier::left_option));
      expect(!modifiers.exists(hid_report::modifier::left_command));
      expect(!modifiers.exists(hid_report::modifier::right_control));
      expect(!modifiers.exists(hid_report::modifier::right_shift));
      expect(!modifiers.exists(hid_report::modifier::right_option));
      expect(!modifiers.exists(hid_report::modifier::right_command));
    }
  };
}
