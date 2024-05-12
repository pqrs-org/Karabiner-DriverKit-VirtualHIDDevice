#include "io_service_client.hpp"
#include "version.hpp"
#include "virtual_hid_device_service_server.hpp"
#include <chrono>
#include <cxxopts.hpp>
#include <iostream>
#include <memory>
#include <pqrs/hid.hpp>
#include <pqrs/local_datagram.hpp>
#include <pqrs/osx/iokit_return.hpp>
#include <pqrs/osx/process_info.hpp>
#include <thread>

extern "C" {
bool service_manager_register();
bool service_manager_unregister();
void service_manager_status();
}

namespace {
int daemon(void) {
  std::signal(SIGINT, SIG_IGN);
  std::signal(SIGTERM, SIG_IGN);
  std::signal(SIGUSR1, SIG_IGN);
  std::signal(SIGUSR2, SIG_IGN);

  pqrs::osx::process_info::enable_sudden_termination();

  pqrs::dispatcher::extra::initialize_shared_dispatcher();
  pqrs::cf::run_loop_thread::extra::initialize_shared_run_loop_thread();

  logger::set_async_rotating_logger("virtual_hid_device_service",
                                    "/var/log/karabiner/virtual_hid_device_service.log",
                                    pqrs::spdlog::filesystem::log_directory_perms_0755);

  logger::get_logger()->info("version {0}", VERSION);

  //
  // Create instances
  //

  auto run_loop_thread = std::make_shared<pqrs::cf::run_loop_thread>();
  auto server = std::make_unique<virtual_hid_device_service_server>(pqrs::cf::run_loop_thread::extra::get_shared_run_loop_thread());

  //
  // Set signal handler
  //

  auto termination_handler = [&server] {
    server = nullptr;

    pqrs::cf::run_loop_thread::extra::terminate_shared_run_loop_thread();
    pqrs::dispatcher::extra::terminate_shared_dispatcher();
    exit(0);
  };

  {
    dispatch_source_t sigint_source = dispatch_source_create(DISPATCH_SOURCE_TYPE_SIGNAL, SIGINT, 0, dispatch_get_main_queue());
    dispatch_source_set_event_handler(sigint_source, ^{
      logger::get_logger()->info("SIGINT");
      termination_handler();
    });
    dispatch_resume(sigint_source);
  }
  {
    dispatch_source_t sigterm_source = dispatch_source_create(DISPATCH_SOURCE_TYPE_SIGNAL, SIGTERM, 0, dispatch_get_main_queue());
    dispatch_source_set_event_handler(sigterm_source, ^{
      logger::get_logger()->info("SIGTERM");
      termination_handler();
    });
    dispatch_resume(sigterm_source);
  }

  //
  // Run
  //

  dispatch_main();

  return 0;
}
} // namespace

int main(int argc, char** argv) {
  cxxopts::Options options("Karabiner-DriverKit-VirtualHIDDeviceClient");

  options.add_options()("daemon",
                        "Run as a daemon");

  options.add_options()("service-management-register",
                        "[Maintenance Command] Register services");

  options.add_options()("service-management-unregister",
                        "[Maintenance Command] Unregister services");

  options.add_options()("service-management-status",
                        "[Maintenance Command] Show services status");

  options.add_options()("version",
                        "Displays version");

  options.add_options()("help",
                        "Print help");

  try {
    auto parse_result = options.parse(argc, argv);

    {
      std::string key = "daemon";
      if (parse_result.count(key)) {
        return daemon();
      }
    }

    {
      std::string key = "service-management-register";
      if (parse_result.count(key)) {
        return service_manager_register() ? 0 : 1;
      }
    }

    {
      std::string key = "service-management-unregister";
      if (parse_result.count(key)) {
        return service_manager_unregister() ? 0 : 1;
      }
    }

    {
      std::string key = "service-management-status";
      if (parse_result.count(key)) {
        service_manager_status();
        return 0;
      }
    }

    {
      std::string key = "version";
      if (parse_result.count(key)) {
        std::cout << "version: " << VERSION << std::endl;
        std::cout << "driver_version: " << DRIVER_VERSION << std::endl;
        std::cout << "client_protocol_version: " << CLIENT_PROTOCOL_VERSION << std::endl;

        return 0;
      }
    }

  } catch (const cxxopts::exceptions::exception& e) {
    std::cout << "error parsing options: " << e.what() << std::endl;
    return 2;
  }

  options.show_positional_help();
  std::cout << options.help() << std::endl;

  return 0;
}
