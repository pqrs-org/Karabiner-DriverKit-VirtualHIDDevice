#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wshorten-64-to-32"
#include <cxxopts.hpp>
#pragma clang diagnostic pop

#include "version.hpp"
#include <pqrs/osx/launch_services.hpp>

int main(int argc, char** argv) {
  int exit_code = 0;

  cxxopts::Options options("cli",
                           "A command line utility of Karabiner-DriverKit-VirtualHIDDevice");

  options.add_options()("lsregister-karabiner-driverkit-virtualhiddeviceclient",
                        "[Maintenance Command] Register Karabiner-DriverKit-VirtualHIDDeviceClient.app in the Launch Services database");

  options.add_options()("version",
                        "Displays version");

  options.add_options()("help",
                        "Print help");

  try {
    auto parse_result = options.parse(argc, argv);

    {
      std::string key = "lsregister-karabiner-driverkit-virtualhiddeviceclient";
      if (parse_result.count(key)) {
        // Update the application name in System Settings > Login Items.
        auto status = pqrs::osx::launch_services::register_application("/Library/Application Support/org.pqrs/Karabiner-DriverKit-VirtualHIDDevice/Applications/Karabiner-DriverKit-VirtualHIDDeviceClient.app");
        std::cout << key << ": " << status.to_string() << std::endl;

        goto finish;
      }
    }

    {
      std::string key = "version";
      if (parse_result.count(key)) {
        std::cout << "version: " << VERSION << std::endl;
        std::cout << "driver_version: " << DRIVER_VERSION << std::endl;
        std::cout << "client_protocol_version: " << CLIENT_PROTOCOL_VERSION << std::endl;

        goto finish;
      }
    }

  } catch (const cxxopts::exceptions::exception& e) {
    std::cout << "error parsing options: " << e.what() << std::endl;
    exit_code = 2;
    goto finish;
  }

  options.show_positional_help();
  std::cout << options.help() << std::endl;

  exit_code = 1;

finish:
  return exit_code;
}
