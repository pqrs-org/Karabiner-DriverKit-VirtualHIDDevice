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

  std::filesystem::path client_socket_file_path1("/tmp/karabiner_driverkit_virtual_hid_device_service_client1.sock");
  std::filesystem::path client_socket_file_path2("/tmp/karabiner_driverkit_virtual_hid_device_service_client2.sock");

  std::mutex client_mutex;
  auto client1 = std::make_unique<pqrs::karabiner::driverkit::virtual_hid_device_service::client>(client_socket_file_path1);
  auto client2 = std::make_unique<pqrs::karabiner::driverkit::virtual_hid_device_service::client>(client_socket_file_path2);

  std::thread call_ready_thread([&client1, &client2, &client_mutex] {
    while (!exit_flag) {
      {
        std::lock_guard<std::mutex> lock(client_mutex);

        if (client1) {
          client1->async_driver_loaded();
          client1->async_driver_version_matched();
          client1->async_virtual_hid_keyboard_ready();
          client1->async_virtual_hid_pointing_ready();
        }

        if (client2) {
          client2->async_driver_loaded();
          client2->async_driver_version_matched();
          client2->async_virtual_hid_keyboard_ready();
          client2->async_virtual_hid_pointing_ready();
        }
      }

      std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
  });

  std::mutex keyboard_thread_mutex;
  std::unique_ptr<std::thread> keyboard_thread1;
  std::unique_ptr<std::thread> keyboard_thread2;

  std::mutex pointing_thread_mutex;
  std::unique_ptr<std::thread> pointing_thread1;

  //
  // client1
  //

  client1->connected.connect([&client1] {
    std::cout << "connected" << std::endl;

    client1->async_virtual_hid_keyboard_initialize(pqrs::hid::country_code::us);
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
  client1->driver_loaded_response.connect([](auto&& driver_loaded) {
    static std::optional<bool> previous_value;

    if (previous_value != driver_loaded) {
      std::cout << "driver_loaded " << driver_loaded << std::endl;
      previous_value = driver_loaded;
    }
  });
  client1->driver_version_matched_response.connect([](auto&& driver_version_matched) {
    static std::optional<bool> previous_value;

    if (previous_value != driver_version_matched) {
      std::cout << "driver_version_matched " << driver_version_matched << std::endl;
      previous_value = driver_version_matched;
    }
  });
  client1->virtual_hid_keyboard_ready_response.connect([&client1, &client_mutex, &keyboard_thread1, &keyboard_thread_mutex](auto&& ready) {
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
            }
          }
        });
      }
    }
  });
  client1->virtual_hid_pointing_ready_response.connect([&client1, &client_mutex, &pointing_thread1, &pointing_thread_mutex](auto&& ready) {
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
    client2->async_virtual_hid_keyboard_initialize(pqrs::hid::country_code::us);
  });
  client2->virtual_hid_keyboard_ready_response.connect([&client2, &client_mutex, &keyboard_thread2, &keyboard_thread_mutex](auto&& ready) {
    if (ready) {
      std::lock_guard<std::mutex> lock(keyboard_thread_mutex);

      if (!keyboard_thread2) {
        keyboard_thread2 = std::make_unique<std::thread>([&client2, &client_mutex] {
          std::this_thread::sleep_for(std::chrono::milliseconds(1000));

          {
            std::lock_guard<std::mutex> lock(client_mutex);

            if (client2) {
              {
                pqrs::karabiner::driverkit::virtual_hid_device_driver::hid_report::do_not_disturb_input report;
                report.do_not_disturb = 0xff;
                client2->async_post_report(report);
              }
              {
                pqrs::karabiner::driverkit::virtual_hid_device_driver::hid_report::do_not_disturb_input report;
                client2->async_post_report(report);
              }

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

  call_ready_thread.join();

  // Needed after using `pqrs::karabiner::driverkit::virtual_hid_device_service::client`.
  pqrs::dispatcher::extra::terminate_shared_dispatcher();

  return 0;
}
