#include "io_service_client.hpp"
#include "version.hpp"
#include "virtual_hid_device_service_server.hpp"
#include <chrono>
#include <iostream>
#include <memory>
#include <pqrs/hid.hpp>
#include <pqrs/local_datagram.hpp>
#include <pqrs/osx/iokit_return.hpp>
#include <pqrs/osx/process_info.hpp>
#include <thread>

namespace {
auto global_wait = pqrs::make_thread_wait();
}

int main(void) {
  std::signal(SIGINT, [](int) {
    global_wait->notify();
  });
  std::signal(SIGUSR1, SIG_IGN);
  std::signal(SIGUSR2, SIG_IGN);

  pqrs::osx::process_info::enable_sudden_termination();

  pqrs::dispatcher::extra::initialize_shared_dispatcher();

  logger::set_async_rotating_logger("virtual_hid_device_service",
                                    "/var/log/karabiner/virtual_hid_device_service.log",
                                    pqrs::spdlog::filesystem::log_directory_perms_0755);

  logger::get_logger()->info("version {0}", VERSION);

  auto server = std::make_unique<virtual_hid_device_service_server>();

  global_wait->wait_notice();

  server = nullptr;

  pqrs::dispatcher::extra::terminate_shared_dispatcher();

  return 0;
}
