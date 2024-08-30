#include <atomic>
#include <filesystem>
#include <iostream>
#include <pqrs/karabiner/driverkit/virtual_hid_device_driver.hpp>
#include <pqrs/karabiner/driverkit/virtual_hid_device_service.hpp>
#include <pqrs/local_datagram.hpp>
#include <thread>

namespace {
std::atomic<bool> exit_flag(false);
}

int main(void) {
  std::signal(SIGINT, [](int) {
    exit_flag = true;
  });

  // Needed before using `pqrs::karabiner::driverkit::virtual_hid_device_service::client`.
  pqrs::dispatcher::extra::initialize_shared_dispatcher();

  std::mutex client_mutex;
  auto client1 = std::make_unique<pqrs::karabiner::driverkit::virtual_hid_device_service::client>();
  auto client2 = std::make_unique<pqrs::karabiner::driverkit::virtual_hid_device_service::client>();

  std::mutex keyboard_thread_mutex;
  std::unique_ptr<std::thread> keyboard_thread1;
  std::unique_ptr<std::thread> keyboard_thread2;

  std::mutex pointing_thread_mutex;
  std::unique_ptr<std::thread> pointing_thread1;

  //
  // client1
  //

  client1->warning_reported.connect([](auto&& message) {
    std::cout << "warning: " << message << std::endl;
  });

  client1->connected.connect([&client1] {
    std::cout << "connected" << std::endl;

    pqrs::karabiner::driverkit::virtual_hid_device_service::virtual_hid_keyboard_parameters parameters;
    parameters.set_country_code(pqrs::hid::country_code::us);

    client1->async_virtual_hid_keyboard_initialize(parameters);
    client1->async_virtual_hid_pointing_initialize();
  });
  client1->connect_failed.connect([](auto&& error_code) {
    std::cout << "connect_failed " << error_code << std::endl;
  });
  client1->closed.connect([] {
    std::cout << "closed" << std::endl;
  });
  client1->error_occurred.connect([](auto&& error_code) {
    std::cout << "error_occurred " << error_code << std::endl;
  });
  client1->driver_activated.connect([](auto&& driver_activated) {
    static std::optional<bool> previous_value;

    if (previous_value != driver_activated) {
      std::cout << "driver_activated " << driver_activated << std::endl;
      previous_value = driver_activated;
    }
  });
  client1->driver_connected.connect([](auto&& driver_connected) {
    static std::optional<bool> previous_value;

    if (previous_value != driver_connected) {
      std::cout << "driver_connected " << driver_connected << std::endl;
      previous_value = driver_connected;
    }
  });
  client1->driver_version_mismatched.connect([](auto&& driver_version_mismatched) {
    static std::optional<bool> previous_value;

    if (previous_value != driver_version_mismatched) {
      std::cout << "driver_version_mismatched " << driver_version_mismatched << std::endl;
      previous_value = driver_version_mismatched;
    }
  });
  client1->virtual_hid_keyboard_ready.connect([&client1, &client_mutex, &keyboard_thread1, &keyboard_thread_mutex](auto&& ready) {
    if (!keyboard_thread1) {
      std::cout << "virtual_hid_keyboard_ready " << ready << std::endl;
    }

    if (ready) {
      std::lock_guard<std::mutex> lock(keyboard_thread_mutex);

      if (!keyboard_thread1) {
        keyboard_thread1 = std::make_unique<std::thread>([&client1, &client_mutex] {
          std::this_thread::sleep_for(std::chrono::milliseconds(500));

          {
            std::lock_guard<std::mutex> lock(client_mutex);

            if (client1) {
              // key down (shift+e)
              {
                pqrs::karabiner::driverkit::virtual_hid_device_driver::hid_report::keyboard_input report;
                report.modifiers.insert(pqrs::karabiner::driverkit::virtual_hid_device_driver::hid_report::modifier::left_shift);
                report.keys.insert(type_safe::get(pqrs::hid::usage::keyboard_or_keypad::keyboard_e));
                client1->async_post_report(report);
              }

#if 0
              // key up
              {
                pqrs::karabiner::driverkit::virtual_hid_device_driver::hid_report::keyboard_input report;
                client1->async_post_report(report);
              }
#endif

              // key down (dpad down)
              {
                pqrs::karabiner::driverkit::virtual_hid_device_driver::hid_report::generic_desktop_input report;
                report.keys.insert(type_safe::get(pqrs::hid::usage::generic_desktop::dpad_down));
                client1->async_post_report(report);
              }

#if 0
              // key up
              {
                pqrs::karabiner::driverkit::virtual_hid_device_driver::hid_report::generic_desktop_input report;
                client1->async_post_report(report);
              }
#endif
            }
          }
        });
      }
    }
  });
  client1->virtual_hid_pointing_ready.connect([&client1, &client_mutex, &pointing_thread1, &pointing_thread_mutex](auto&& ready) {
    if (!pointing_thread1) {
      std::cout << "virtual_hid_pointing_ready " << ready << std::endl;
    }

    if (ready) {
      std::lock_guard<std::mutex> lock(pointing_thread_mutex);

      if (!pointing_thread1) {
        pointing_thread1 = std::make_unique<std::thread>([&client1, &client_mutex] {
          for (int i = 0; i < 400; ++i) {
            {
              std::lock_guard<std::mutex> lock(client_mutex);

              if (client1) {
                pqrs::karabiner::driverkit::virtual_hid_device_driver::hid_report::pointing_input report;
                report.x = static_cast<uint8_t>(cos(0.1 * i) * 20);
                report.y = static_cast<uint8_t>(sin(0.1 * i) * 20);
                client1->async_post_report(report);
              }
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(5));
          }
        });
      }
    }
  });

  client1->async_start();

  //
  // client2
  //

  client2->connected.connect([&client2] {
    pqrs::karabiner::driverkit::virtual_hid_device_service::virtual_hid_keyboard_parameters parameters;
    parameters.set_country_code(pqrs::hid::country_code::us);

    client2->async_virtual_hid_keyboard_initialize(parameters);
  });
  client2->virtual_hid_keyboard_ready.connect([&client2, &client_mutex, &keyboard_thread2, &keyboard_thread_mutex](auto&& ready) {
    if (ready) {
      std::lock_guard<std::mutex> lock(keyboard_thread_mutex);

      if (!keyboard_thread2) {
        keyboard_thread2 = std::make_unique<std::thread>([&client2, &client_mutex] {
          std::this_thread::sleep_for(std::chrono::milliseconds(1000));

          {
            std::lock_guard<std::mutex> lock(client_mutex);

            if (client2) {
              // key down (g)
              {
                pqrs::karabiner::driverkit::virtual_hid_device_driver::hid_report::keyboard_input report;
                report.keys.insert(type_safe::get(pqrs::hid::usage::keyboard_or_keypad::keyboard_g));
                client2->async_post_report(report);
              }

#if 0
              // key up
              {
                pqrs::karabiner::driverkit::virtual_hid_device_driver::hid_report::keyboard_input report;
                client2->async_post_report(report);
              }
#endif
            }
          }
        });
      }
    }
  });

  client2->async_start();

  //
  // Wait control-c
  //

  std::cout << std::endl;
  std::cout << "Press control+c to quit." << std::endl;
  std::cout << std::endl;

  std::cout << "pqrs::karabiner::driverkit::driver_version::embedded_driver_version: " << pqrs::karabiner::driverkit::driver_version::embedded_driver_version << std::endl;
  std::cout << "pqrs::karabiner::driverkit::client_protocol_version::embedded_client_protocol_version: " << pqrs::karabiner::driverkit::client_protocol_version::embedded_client_protocol_version << std::endl;

  while (!exit_flag) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  //
  // Termination
  //

  std::this_thread::sleep_for(std::chrono::milliseconds(500));

  {
    std::lock_guard<std::mutex> lock(client_mutex);
    client1 = nullptr;
    client2 = nullptr;
  }

  {
    std::lock_guard<std::mutex> lock(keyboard_thread_mutex);
    if (keyboard_thread1) {
      keyboard_thread1->join();
    }
    if (keyboard_thread2) {
      keyboard_thread2->join();
    }
  }

  {
    std::lock_guard<std::mutex> lock(pointing_thread_mutex);
    if (pointing_thread1) {
      pointing_thread1->join();
    }
  }

  // Needed after using `pqrs::karabiner::driverkit::virtual_hid_device_service::client`.
  pqrs::dispatcher::extra::terminate_shared_dispatcher();

  return 0;
}
