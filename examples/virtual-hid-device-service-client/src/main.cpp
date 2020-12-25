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

  std::filesystem::path client_socket_file_path("/tmp/karabiner_driverkit_virtual_hid_device_service_client.sock");

  std::mutex client_mutex;
  auto client = std::make_unique<pqrs::karabiner::driverkit::virtual_hid_device_service::client>(client_socket_file_path);

  std::thread call_ready_thread([&client, &client_mutex] {
    while (!exit_flag) {
      {
        std::lock_guard<std::mutex> lock(client_mutex);

        if (client) {
          client->async_driver_loaded();
          client->async_driver_version_matched();
          client->async_virtual_hid_keyboard_ready();
          client->async_virtual_hid_pointing_ready();
        }
      }

      std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
  });

  std::mutex keyboard_thread_mutex;
  std::unique_ptr<std::thread> keyboard_thread;

  std::mutex pointing_thread_mutex;
  std::unique_ptr<std::thread> pointing_thread;

  client->connected.connect([&client] {
    std::cout << "connected" << std::endl;

    client->async_virtual_hid_keyboard_initialize(pqrs::hid::country_code::us);
    client->async_virtual_hid_pointing_initialize();
  });
  client->connect_failed.connect([](auto&& error_code) {
    std::cout << "connect_failed " << error_code << std::endl;
  });
  client->closed.connect([] {
    std::cout << "closed" << std::endl;
  });
  client->error_occurred.connect([](auto&& error_code) {
    std::cout << "error_occurred " << error_code << std::endl;
  });
  client->driver_loaded_response.connect([](auto&& driver_loaded) {
    static std::optional<bool> previous_value;

    if (previous_value != driver_loaded) {
      std::cout << "driver_loaded " << driver_loaded << std::endl;
      previous_value = driver_loaded;
    }
  });
  client->driver_version_matched_response.connect([](auto&& driver_version_matched) {
    static std::optional<bool> previous_value;

    if (previous_value != driver_version_matched) {
      std::cout << "driver_version_matched " << driver_version_matched << std::endl;
      previous_value = driver_version_matched;
    }
  });
  client->virtual_hid_keyboard_ready_response.connect([&client, &client_mutex, &keyboard_thread, &keyboard_thread_mutex](auto&& ready) {
    if (!keyboard_thread) {
      std::cout << "virtual_hid_keyboard_ready " << ready << std::endl;
    }

    if (ready) {
      std::lock_guard<std::mutex> lock(keyboard_thread_mutex);

      if (!keyboard_thread) {
        keyboard_thread = std::make_unique<std::thread>([&client, &client_mutex] {
          std::this_thread::sleep_for(std::chrono::milliseconds(500));

          {
            std::lock_guard<std::mutex> lock(client_mutex);

            if (client) {
              // key down (control+up)
              {
                pqrs::karabiner::driverkit::virtual_hid_device_driver::hid_report::keyboard_input report;
                report.modifiers.insert(pqrs::karabiner::driverkit::virtual_hid_device_driver::hid_report::modifier::left_control);
                report.keys.insert(type_safe::get(pqrs::hid::usage::keyboard_or_keypad::keyboard_up_arrow));
                client->async_post_report(report);
              }

              // key up
              {
                pqrs::karabiner::driverkit::virtual_hid_device_driver::hid_report::keyboard_input report;
                client->async_post_report(report);
              }
            }
          }
        });
      }
    }
  });
  client->virtual_hid_pointing_ready_response.connect([&client, &client_mutex, &pointing_thread, &pointing_thread_mutex](auto&& ready) {
    if (!pointing_thread) {
      std::cout << "virtual_hid_pointing_ready " << ready << std::endl;
    }

    if (ready) {
      std::lock_guard<std::mutex> lock(pointing_thread_mutex);

      if (!pointing_thread) {
        pointing_thread = std::make_unique<std::thread>([&client, &client_mutex] {
          for (int i = 0; i < 400; ++i) {
            {
              std::lock_guard<std::mutex> lock(client_mutex);

              if (client) {
                pqrs::karabiner::driverkit::virtual_hid_device_driver::hid_report::pointing_input report;
                report.x = static_cast<uint8_t>(cos(0.1 * i) * 20);
                report.y = static_cast<uint8_t>(sin(0.1 * i) * 20);
                client->async_post_report(report);
              }
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(5));
          }
        });
      }
    }
  });

  client->async_start();

  //
  // Wait control-c
  //

  std::cout << "Press control+c to quit." << std::endl;

  while (!exit_flag) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  //
  // Termination
  //

  client->async_virtual_hid_keyboard_terminate();
  client->async_virtual_hid_pointing_terminate();

  std::this_thread::sleep_for(std::chrono::milliseconds(500));

  {
    std::lock_guard<std::mutex> lock(client_mutex);
    client = nullptr;
  }

  {
    std::lock_guard<std::mutex> lock(keyboard_thread_mutex);
    if (keyboard_thread) {
      keyboard_thread->join();
    }
  }

  {
    std::lock_guard<std::mutex> lock(pointing_thread_mutex);
    if (pointing_thread) {
      pointing_thread->join();
    }
  }

  call_ready_thread.join();

  // Needed after using `pqrs::karabiner::driverkit::virtual_hid_device_service::client`.
  pqrs::dispatcher::extra::terminate_shared_dispatcher();

  return 0;
}
