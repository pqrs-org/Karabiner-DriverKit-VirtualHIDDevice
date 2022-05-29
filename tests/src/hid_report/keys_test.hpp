#include <boost/ut.hpp>
#include <pqrs/karabiner/driverkit/virtual_hid_device_driver.hpp>

void run_keys_test(void) {
  using namespace boost::ut;
  using namespace boost::ut::literals;

  "keys"_test = [] {
    using namespace pqrs::karabiner::driverkit::virtual_hid_device_driver;

    {
      hid_report::keys keys;
      uint16_t expected[32];

      expect(keys.count() == 0);
      expect(keys.empty());
      memset(expected, 0, sizeof(expected));
      expect(memcmp(keys.get_raw_value(), expected, sizeof(expected)) == 0);

      keys.insert(10);
      expect(keys.count() == 1);
      expect(!keys.empty());
      expect(keys.exists(10));
      expect(!keys.exists(20));
      expected[0] = 10;
      expect(memcmp(keys.get_raw_value(), expected, sizeof(expected)) == 0);

      keys.insert(10);
      expect(keys.count() == 1);
      expect(!keys.empty());
      expect(memcmp(keys.get_raw_value(), expected, sizeof(expected)) == 0);

      keys.erase(20);
      expect(keys.count() == 1);
      expect(!keys.empty());
      expect(memcmp(keys.get_raw_value(), expected, sizeof(expected)) == 0);

      keys.erase(10);
      expect(keys.count() == 0);
      expect(keys.empty());
      expected[0] = 0;
      expect(memcmp(keys.get_raw_value(), expected, sizeof(expected)) == 0);

      keys.erase(10);
      expect(keys.count() == 0);
      expect(keys.empty());
      expect(memcmp(keys.get_raw_value(), expected, sizeof(expected)) == 0);

      keys.insert(10);
      expect(keys.count() == 1);
      expect(!keys.empty());
      expected[0] = 10;
      expect(memcmp(keys.get_raw_value(), expected, sizeof(expected)) == 0);

      keys.insert(20);
      expect(keys.count() == 2);
      expect(!keys.empty());
      expected[1] = 20;
      expect(memcmp(keys.get_raw_value(), expected, sizeof(expected)) == 0);

      keys.clear();
      expect(keys.count() == 0);
      expect(keys.empty());
      expected[0] = 0;
      expected[1] = 0;
      expect(memcmp(keys.get_raw_value(), expected, sizeof(expected)) == 0);
    }

    {
      // Overflow

      hid_report::keys keys;
      expect(keys.count() == 0);

      for (int i = 0; i < 32; ++i) {
        keys.insert(i + 1);
        expect(keys.count() == (i + 1));
      }

      keys.insert(10);
      expect(keys.count() == 32);

      keys.insert(20);
      expect(keys.count() == 32);
    }
  };
}
