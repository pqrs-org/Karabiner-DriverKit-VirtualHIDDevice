#pragma once

#include "logger.hpp"
#include <filesystem>
#include <nod/nod.hpp>
#include <pqrs/dispatcher.hpp>
#include <pqrs/karabiner/driverkit/virtual_hid_device_service.hpp>
#include <pqrs/local_datagram.hpp>
#include <unordered_map>

class virtual_hid_device_service_clients_manager final : public pqrs::dispatcher::extra::dispatcher_client {
public:
  virtual_hid_device_service_clients_manager(std::shared_ptr<pqrs::cf::run_loop_thread> run_loop_thread)
      : dispatcher_client(),
        run_loop_thread_(run_loop_thread) {
  }

  virtual ~virtual_hid_device_service_clients_manager(void) {
    detach_from_dispatcher([this] {
      entries_.clear();
    });
  }

  // This method needs to be called in the dispatcher thread.
  void create_client(const std::string& endpoint_path) {
    if (!dispatcher_thread()) {
      throw std::logic_error(fmt::format("{0} is called in wrong thread", __func__));
    }

    auto endpoint_filename = std::filesystem::path(endpoint_path).filename();

    {
      auto it = entries_.find(endpoint_path);
      if (it != std::end(entries_)) {
        logger::get_logger()->info(
            "{0} client already exists",
            endpoint_filename.c_str());
        return;
      }
    }

    logger::get_logger()->info(
        "{0} create a client for virtual_hid_device_service::client",
        endpoint_filename.c_str());

    //
    // Create pqrs::local_datagram::client
    //

    auto c = std::make_shared<pqrs::local_datagram::client>(
        weak_dispatcher_,
        endpoint_path,
        server_response_socket_file_path(),
        pqrs::karabiner::driverkit::virtual_hid_device_service::constants::local_datagram_buffer_size);

    c->set_server_check_interval(std::chrono::milliseconds(1000));
    c->set_next_heartbeat_deadline(std::chrono::milliseconds(5000));

    c->warning_reported.connect([endpoint_filename](auto&& message) {
      logger::get_logger()->warn(
          "{0} client: {1}",
          endpoint_filename.c_str(),
          message);
    });

    c->connect_failed.connect([this, endpoint_path, endpoint_filename](auto&& error_code) {
      logger::get_logger()->info(
          "{0} client connect_failed",
          endpoint_filename.c_str());

      erase_client(endpoint_path);
    });

    c->closed.connect([this, endpoint_path, endpoint_filename] {
      logger::get_logger()->info(
          "{0} client closed",
          endpoint_filename.c_str());

      erase_client(endpoint_path);
    });

    c->async_start();

    entries_[endpoint_path] = std::make_unique<entry>(c,
                                                      run_loop_thread_,
                                                      endpoint_filename);

    logger::get_logger()->info("{0} virtual_hid_device_service_clients_manager client is added (size: {1})",
                               endpoint_filename.c_str(),
                               entries_.size());
  }

  // This method needs to be called in the dispatcher thread.
  void erase_client(const std::string& endpoint_path) {
    if (!dispatcher_thread()) {
      throw std::logic_error(fmt::format("{0} is called in wrong thread", __func__));
    }

    auto endpoint_filename = std::filesystem::path(endpoint_path).filename();

    entries_.erase(endpoint_path);

    logger::get_logger()->info("{0} virtual_hid_device_service_clients_manager client is removed (size: {1})",
                               endpoint_filename.c_str(),
                               entries_.size());
  }

  void initialize_keyboard(const std::string& endpoint_path,
                           const pqrs::karabiner::driverkit::virtual_hid_device_service::virtual_hid_keyboard_parameters& parameters) {
    if (!dispatcher_thread()) {
      throw std::logic_error(fmt::format("{0} is called in wrong thread", __func__));
    }

    auto it = entries_.find(endpoint_path);
    if (it != std::end(entries_)) {
      it->second->initialize_keyboard(parameters);
    }
  }

  void terminate_keyboard(const std::string& endpoint_path) {
    if (!dispatcher_thread()) {
      throw std::logic_error(fmt::format("{0} is called in wrong thread", __func__));
    }

    auto it = entries_.find(endpoint_path);
    if (it != std::end(entries_)) {
      it->second->terminate_keyboard();
    }
  }

  void initialize_pointing(const std::string& endpoint_path) {
    if (!dispatcher_thread()) {
      throw std::logic_error(fmt::format("{0} is called in wrong thread", __func__));
    }

    auto it = entries_.find(endpoint_path);
    if (it != std::end(entries_)) {
      it->second->initialize_pointing();
    }
  }

  void terminate_pointing(const std::string& endpoint_path) {
    if (!dispatcher_thread()) {
      throw std::logic_error(fmt::format("{0} is called in wrong thread", __func__));
    }

    auto it = entries_.find(endpoint_path);
    if (it != std::end(entries_)) {
      it->second->terminate_pointing();
    }
  }

  // This method needs to be called in the dispatcher thread.
  void virtual_hid_keyboard_reset(const std::string& endpoint_path) const {
    if (!dispatcher_thread()) {
      throw std::logic_error(fmt::format("{0} is called in wrong thread", __func__));
    }

    auto it = entries_.find(endpoint_path);
    if (it != std::end(entries_)) {
      if (auto c = it->second->get_io_service_client_keyboard()) {
        c->async_virtual_hid_keyboard_reset();
      }
    }
  }

  // This method needs to be called in the dispatcher thread.
  void virtual_hid_pointing_reset(const std::string& endpoint_path) const {
    if (!dispatcher_thread()) {
      throw std::logic_error(fmt::format("{0} is called in wrong thread", __func__));
    }

    auto it = entries_.find(endpoint_path);
    if (it != std::end(entries_)) {
      if (auto c = it->second->get_io_service_client_pointing()) {
        c->async_virtual_hid_pointing_reset();
      }
    }
  }

  // This method needs to be called in the dispatcher thread.
  template <typename T>
  void post_keyboard_report(const std::string& endpoint_path,
                            const uint8_t* buffer,
                            size_t buffer_size) const {
    if (!dispatcher_thread()) {
      throw std::logic_error(fmt::format("{0} is called in wrong thread", __func__));
    }

    if (sizeof(T) != buffer_size) {
      logger::get_logger()->warn(fmt::format("{0}: buffer size error", __func__));
      return;
    }

    auto it = entries_.find(endpoint_path);
    if (it != std::end(entries_)) {
      if (auto c = it->second->get_io_service_client_keyboard()) {
        c->async_post_report(*(reinterpret_cast<const T*>(buffer)));
      }
    }
  }

  // This method needs to be called in the dispatcher thread.
  template <typename T>
  void post_pointing_report(const std::string& endpoint_path,
                            const uint8_t* buffer,
                            size_t buffer_size) const {
    if (!dispatcher_thread()) {
      throw std::logic_error(fmt::format("{0} is called in wrong thread", __func__));
    }

    if (sizeof(T) != buffer_size) {
      logger::get_logger()->warn(fmt::format("{0}: buffer size error", __func__));
      return;
    }

    auto it = entries_.find(endpoint_path);
    if (it != std::end(entries_)) {
      if (auto c = it->second->get_io_service_client_pointing()) {
        c->async_post_report(*(reinterpret_cast<const T*>(buffer)));
      }
    }
  }

private:
  class entry final : public pqrs::dispatcher::extra::dispatcher_client {
  public:
    entry(std::shared_ptr<pqrs::local_datagram::client> local_datagram_client,
          std::shared_ptr<pqrs::cf::run_loop_thread> run_loop_thread,
          const std::string& virtual_hid_device_service_client_endpoint_filename)
        : local_datagram_client_(local_datagram_client),
          run_loop_thread_(run_loop_thread),
          initialize_timer_(*this),
          ready_timer_(*this),
          virtual_hid_keyboard_enabled_(false),
          virtual_hid_pointing_enabled_(false) {
      io_service_client_nop_ = std::make_shared<io_service_client>(run_loop_thread_,
                                                                   virtual_hid_device_service_client_endpoint_filename);
      io_service_client_nop_->async_start();

      initialize_timer_.start(
          [this, virtual_hid_device_service_client_endpoint_filename] {
            //
            // Setup virtual_hid_keyboard
            //

            if (virtual_hid_keyboard_enabled_) {
              if (!virtual_hid_keyboard_ready()) {
                io_service_client_keyboard_ = std::make_shared<io_service_client>(run_loop_thread_,
                                                                                  virtual_hid_device_service_client_endpoint_filename);

                io_service_client_keyboard_->opened.connect([this] {
                  io_service_client_keyboard_->async_virtual_hid_keyboard_initialize(virtual_hid_keyboard_parameters_);
                });

                io_service_client_keyboard_->async_start();
              }
            } else {
              io_service_client_keyboard_ = nullptr;
            }

            //
            // Setup virtual_hid_pointing
            //

            if (virtual_hid_pointing_enabled_) {
              if (!virtual_hid_pointing_ready()) {
                io_service_client_pointing_ = std::make_shared<io_service_client>(run_loop_thread_,
                                                                                  virtual_hid_device_service_client_endpoint_filename);

                io_service_client_pointing_->opened.connect([this] {
                  io_service_client_pointing_->async_virtual_hid_pointing_initialize();
                });

                io_service_client_pointing_->async_start();
              }
            } else {
              io_service_client_pointing_ = nullptr;
            }
          },
          // The call interval of `initialize_timer_` must be longer than that of `ready_timer_`.
          std::chrono::milliseconds(5000));

      ready_timer_.start(
          [this] {
            //
            // Query `ready` state to driver
            //

            if (auto c = io_service_client_keyboard_) {
              c->async_virtual_hid_keyboard_ready();
            }
            if (auto c = io_service_client_pointing_) {
              c->async_virtual_hid_pointing_ready();
            }

            //
            // Send state to client
            //

            async_send_driver_activated();
            async_send_driver_connected();
            async_send_driver_version_mismatched();
            async_send_ready(pqrs::karabiner::driverkit::virtual_hid_device_service::response::virtual_hid_keyboard_ready,
                             virtual_hid_keyboard_ready());
            async_send_ready(pqrs::karabiner::driverkit::virtual_hid_device_service::response::virtual_hid_pointing_ready,
                             virtual_hid_pointing_ready());
          },
          std::chrono::milliseconds(1000));
    }

    ~entry(void) {
      detach_from_dispatcher([this] {
        io_service_client_pointing_ = nullptr;
        io_service_client_keyboard_ = nullptr;
        io_service_client_nop_ = nullptr;
      });
    }

    std::shared_ptr<io_service_client> get_io_service_client_keyboard(void) const {
      return io_service_client_keyboard_;
    }

    std::shared_ptr<io_service_client> get_io_service_client_pointing(void) const {
      return io_service_client_pointing_;
    }

    //
    // io_service_client_keyboard_
    //

    void initialize_keyboard(const pqrs::karabiner::driverkit::virtual_hid_device_service::virtual_hid_keyboard_parameters& parameters) {
      enqueue_to_dispatcher([this, parameters] {
        // Destroy io_service_client_keyboard_ if parameters is changed.
        if (io_service_client_keyboard_ &&
            virtual_hid_keyboard_parameters_ != parameters) {
          logger::get_logger()->info("destroy io_service_client_keyboard_ due to parameter changes");

          io_service_client_keyboard_ = nullptr;
        }

        virtual_hid_keyboard_enabled_ = true;
        virtual_hid_keyboard_parameters_ = parameters;
      });
    }

    void terminate_keyboard(void) {
      enqueue_to_dispatcher([this] {
        virtual_hid_keyboard_enabled_ = false;
      });
    }

    //
    // io_service_client_pointing_
    //

    void initialize_pointing(void) {
      enqueue_to_dispatcher([this] {
        virtual_hid_pointing_enabled_ = true;
      });
    }

    void terminate_pointing(void) {
      enqueue_to_dispatcher([this] {
        virtual_hid_pointing_enabled_ = false;
      });
    }

  private:
    // This method is executed in the dispatcher thread.
    bool virtual_hid_keyboard_ready(void) const {
      std::optional<bool> ready;

      if (io_service_client_keyboard_) {
        ready = io_service_client_keyboard_->get_virtual_hid_keyboard_ready();
      }

      return ready ? *ready : false;
    }

    // This method is executed in the dispatcher thread.
    bool virtual_hid_pointing_ready(void) const {
      std::optional<bool> ready;

      if (io_service_client_pointing_) {
        ready = io_service_client_pointing_->get_virtual_hid_pointing_ready();
      }

      return ready ? *ready : false;
    }

    // This method is executed in the dispatcher thread.
    void async_send_driver_activated(void) const {
      bool driver_activated = io_service_client_nop_->driver_activated();
      auto response = pqrs::karabiner::driverkit::virtual_hid_device_service::response::driver_activated;
      uint8_t buffer[] = {
          static_cast<std::underlying_type<decltype(response)>::type>(response),
          driver_activated,
      };

      local_datagram_client_->async_send(buffer, sizeof(buffer));
    }

    // This method is executed in the dispatcher thread.
    void async_send_driver_connected(void) const {
      bool driver_connected = io_service_client_nop_->driver_connected();
      auto response = pqrs::karabiner::driverkit::virtual_hid_device_service::response::driver_connected;
      uint8_t buffer[] = {
          static_cast<std::underlying_type<decltype(response)>::type>(response),
          driver_connected,
      };

      local_datagram_client_->async_send(buffer, sizeof(buffer));
    }

    // This method is executed in the dispatcher thread.
    void async_send_driver_version_mismatched(void) const {
      bool driver_version_mismatched = io_service_client_nop_->driver_version_mismatched();
      auto response = pqrs::karabiner::driverkit::virtual_hid_device_service::response::driver_version_mismatched;
      uint8_t buffer[] = {
          static_cast<std::underlying_type<decltype(response)>::type>(response),
          driver_version_mismatched,
      };

      local_datagram_client_->async_send(buffer, sizeof(buffer));
    }

    // This method is executed in the dispatcher thread.
    void async_send_ready(pqrs::karabiner::driverkit::virtual_hid_device_service::response response,
                          bool ready) const {
      uint8_t buffer[] = {
          static_cast<std::underlying_type<decltype(response)>::type>(response),
          ready,
      };

      local_datagram_client_->async_send(buffer, sizeof(buffer));
    }

    std::shared_ptr<pqrs::local_datagram::client> local_datagram_client_;
    std::shared_ptr<pqrs::cf::run_loop_thread> run_loop_thread_;

    std::shared_ptr<io_service_client> io_service_client_nop_;
    std::shared_ptr<io_service_client> io_service_client_keyboard_;
    std::shared_ptr<io_service_client> io_service_client_pointing_;
    pqrs::dispatcher::extra::timer initialize_timer_;
    pqrs::dispatcher::extra::timer ready_timer_;

    // virtual_hid_keyboard
    bool virtual_hid_keyboard_enabled_;
    pqrs::karabiner::driverkit::virtual_hid_device_service::virtual_hid_keyboard_parameters virtual_hid_keyboard_parameters_;

    // virtual_hid_pointing
    bool virtual_hid_pointing_enabled_;
  };

  std::filesystem::path server_response_socket_file_path(void) const {
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();

    std::stringstream ss;
    ss << pqrs::karabiner::driverkit::virtual_hid_device_service::constants::get_server_response_socket_directory_path().string()
       << "/"
       << std::hex
       << std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count()
       << ".sock";

    return ss.str();
  }

  std::shared_ptr<pqrs::cf::run_loop_thread> run_loop_thread_;
  std::unordered_map<std::string, std::unique_ptr<entry>> entries_;
};
