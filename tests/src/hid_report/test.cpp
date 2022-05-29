#include "buttons_test.hpp"
#include "keys_test.hpp"
#include "modifiers_test.hpp"
#include "sizeof_test.hpp"

int main(void) {
  run_buttons_test();
  run_keys_test();
  run_modifiers_test();
  run_sizeof_test();
  return 0;
}
