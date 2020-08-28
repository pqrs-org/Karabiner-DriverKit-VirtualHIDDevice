#include <iostream>
#include <pqrs/local_datagram.hpp>

namespace {
auto global_wait = pqrs::make_thread_wait();
}

int main(void) {
  std::signal(SIGINT, [](int) {
    global_wait->notify();
  });

  std::cout << "hello" << std::endl;

  global_wait->wait_notice();

  return 0;
}
