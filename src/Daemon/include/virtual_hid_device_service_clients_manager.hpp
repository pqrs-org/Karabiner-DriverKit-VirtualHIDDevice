#pragma once

#include "logger.hpp"
#include <algorithm>
#include <array>
#include <memory>
#include <nod/nod.hpp>
#include <pqrs/dispatcher.hpp>
#include <pqrs/karabiner/driverkit/virtual_hid_device_service.hpp>
#include <pqrs/unix_domain_stream.hpp>
#include <unordered_map>
#include <utility>
#include <vector>

class virtual_hid_device_service_clients_manager final : public pqrs::dispatcher::extra::dispatcher_client {
public:
  virtual_hid_device_service_clients_manager(pqrs::not_null_shared_ptr_t<pqrs::cf::run_loop_thread> run_loop_thread)
      : dispatcher_client(),
        run_loop_thread_(run_loop_thread) {
  }

  ~virtual_hid_device_service_clients_manager() override {
    detach_from_dispatcher([this] {
      entries_.clear();
    });
  }

  // This method needs to be called in the dispatcher thread.
  void create_client(pqrs::unix_domain_stream::peer_id peer_id) {
    if (!dispatcher_thread()) {
      throw std::logic_error(fmt::format("{0} is called in wrong thread", __func__));
    }

    auto log_label = fmt::format("peer_id:{0}", peer_id);

    if (entries_.contains(peer_id)) {
      logger::get_logger()->info(
          "{0} client already exists",
          log_label);
      return;
    }

    logger::get_logger()->info(
        "{0} create a client for virtual_hid_device_service::client",
        log_label);

    entries_[peer_id] = std::make_unique<entry>(run_loop_thread_,
                                                log_label);

    logger::get_logger()->info("{0} virtual_hid_device_service_clients_manager client is added (size: {1})",
                               log_label,
                               entries_.size());
  }

  // This method needs to be called in the dispatcher thread.
  void erase_client(pqrs::unix_domain_stream::peer_id peer_id) {
    if (!dispatcher_thread()) {
      throw std::logic_error(fmt::format("{0} is called in wrong thread", __func__));
    }

    auto log_label = fmt::format("peer_id:{0}", peer_id);

    entries_.erase(peer_id);

    logger::get_logger()->info("{0} virtual_hid_device_service_clients_manager client is removed (size: {1})",
                               log_label,
                               entries_.size());
  }

  void initialize_keyboard(pqrs::unix_domain_stream::peer_id peer_id,
                           const pqrs::karabiner::driverkit::virtual_hid_device_service::virtual_hid_keyboard_parameters& parameters) {
    if (!dispatcher_thread()) {
      throw std::logic_error(fmt::format("{0} is called in wrong thread", __func__));
    }

    if (auto it = entries_.find(peer_id);
        it != entries_.end()) {
      it->second->initialize_keyboard(parameters);
    }
  }

  void terminate_keyboard(pqrs::unix_domain_stream::peer_id peer_id) {
    if (!dispatcher_thread()) {
      throw std::logic_error(fmt::format("{0} is called in wrong thread", __func__));
    }

    if (auto it = entries_.find(peer_id);
        it != entries_.end()) {
      it->second->terminate_keyboard();
    }
  }

  void initialize_pointing(pqrs::unix_domain_stream::peer_id peer_id) {
    if (!dispatcher_thread()) {
      throw std::logic_error(fmt::format("{0} is called in wrong thread", __func__));
    }

    if (auto it = entries_.find(peer_id);
        it != entries_.end()) {
      it->second->initialize_pointing();
    }
  }

  void terminate_pointing(pqrs::unix_domain_stream::peer_id peer_id) {
    if (!dispatcher_thread()) {
      throw std::logic_error(fmt::format("{0} is called in wrong thread", __func__));
    }

    if (auto it = entries_.find(peer_id);
        it != entries_.end()) {
      it->second->terminate_pointing();
    }
  }

  // This method needs to be called in the dispatcher thread.
  void virtual_hid_keyboard_reset(pqrs::unix_domain_stream::peer_id peer_id) const {
    if (!dispatcher_thread()) {
      throw std::logic_error(fmt::format("{0} is called in wrong thread", __func__));
    }

    if (auto it = entries_.find(peer_id);
        it != entries_.end()) {
      if (auto client = it->second->get_io_service_client_keyboard()) {
        client->async_virtual_hid_keyboard_reset();
      }
    }
  }

  // This method needs to be called in the dispatcher thread.
  void virtual_hid_pointing_reset(pqrs::unix_domain_stream::peer_id peer_id) const {
    if (!dispatcher_thread()) {
      throw std::logic_error(fmt::format("{0} is called in wrong thread", __func__));
    }

    if (auto it = entries_.find(peer_id);
        it != entries_.end()) {
      if (auto client = it->second->get_io_service_client_pointing()) {
        client->async_virtual_hid_pointing_reset();
      }
    }
  }

  // This method needs to be called in the dispatcher thread.
  std::vector<uint8_t> make_response(pqrs::unix_domain_stream::peer_id peer_id) const {
    if (!dispatcher_thread()) {
      throw std::logic_error(fmt::format("{0} is called in wrong thread", __func__));
    }

    if (auto it = entries_.find(peer_id);
        it != entries_.end()) {
      return it->second->make_response();
    }

    return {};
  }

  // This method needs to be called in the dispatcher thread.
  void post_keyboard_report(pqrs::unix_domain_stream::peer_id peer_id,
                            std::shared_ptr<std::vector<uint8_t>> buffer,
                            size_t report_offset,
                            pqrs::karabiner::driverkit::virtual_hid_device_driver::user_client_method user_client_method,
                            const char* report_name,
                            size_t expected_size) const {
    post_report(peer_id,
                std::move(buffer),
                report_offset,
                user_client_method,
                report_name,
                expected_size,
                [](const entry& entry) {
                  return entry.get_io_service_client_keyboard();
                });
  }

  // This method needs to be called in the dispatcher thread.
  void post_pointing_report(pqrs::unix_domain_stream::peer_id peer_id,
                            std::shared_ptr<std::vector<uint8_t>> buffer,
                            size_t report_offset,
                            pqrs::karabiner::driverkit::virtual_hid_device_driver::user_client_method user_client_method,
                            const char* report_name,
                            size_t expected_size) const {
    post_report(peer_id,
                std::move(buffer),
                report_offset,
                user_client_method,
                report_name,
                expected_size,
                [](const entry& entry) {
                  return entry.get_io_service_client_pointing();
                });
  }

private:
  class entry final : public pqrs::dispatcher::extra::dispatcher_client {
  public:
    entry(pqrs::not_null_shared_ptr_t<pqrs::cf::run_loop_thread> run_loop_thread,
          const std::string& log_label)
        : run_loop_thread_(run_loop_thread),
          log_label_(log_label),
          ready_timer_(*this),
          virtual_hid_keyboard_client_generation_id_(0),
          virtual_hid_keyboard_enabled_(false),
          virtual_hid_pointing_client_generation_id_(0),
          virtual_hid_pointing_enabled_(false) {
      io_service_client_no_virtual_devices_ = std::make_shared<io_service_client>(run_loop_thread_,
                                                                                  log_label);
      io_service_client_no_virtual_devices_->async_start();

      ready_timer_.start(
          [this] {
            //
            // Query `ready` state to driver
            //

            if (auto client = io_service_client_keyboard_) {
              client->async_virtual_hid_keyboard_ready();
            }
            if (auto client = io_service_client_pointing_) {
              client->async_virtual_hid_pointing_ready();
            }
          },
          std::chrono::milliseconds(1000));
    }

    ~entry() {
      detach_from_dispatcher([this] {
        io_service_client_pointing_ = nullptr;
        io_service_client_keyboard_ = nullptr;
        io_service_client_no_virtual_devices_ = nullptr;
      });
    }

    std::shared_ptr<io_service_client> get_io_service_client_keyboard() const {
      return io_service_client_keyboard_;
    }

    std::shared_ptr<io_service_client> get_io_service_client_pointing() const {
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

        setup_virtual_hid_devices();
      });
    }

    void terminate_keyboard() {
      enqueue_to_dispatcher([this] {
        virtual_hid_keyboard_enabled_ = false;

        ++virtual_hid_keyboard_client_generation_id_;
        io_service_client_keyboard_ = nullptr;
      });
    }

    //
    // io_service_client_pointing_
    //

    void initialize_pointing() {
      enqueue_to_dispatcher([this] {
        virtual_hid_pointing_enabled_ = true;

        setup_virtual_hid_devices();
      });
    }

    void terminate_pointing() {
      enqueue_to_dispatcher([this] {
        virtual_hid_pointing_enabled_ = false;

        ++virtual_hid_pointing_client_generation_id_;
        io_service_client_pointing_ = nullptr;
      });
    }

    std::vector<uint8_t> make_response() const {
      std::vector<uint8_t> buffer;
      using response = pqrs::karabiner::driverkit::virtual_hid_device_service::response;

      std::ranges::for_each(
          std::array{
              std::pair{response::driver_activated, io_service_client_no_virtual_devices_->driver_activated()},
              std::pair{response::driver_connected, io_service_client_no_virtual_devices_->driver_connected()},
              std::pair{response::driver_version_mismatched, io_service_client_no_virtual_devices_->driver_version_mismatched()},
              std::pair{response::virtual_hid_keyboard_ready, virtual_hid_keyboard_ready()},
              std::pair{response::virtual_hid_pointing_ready, virtual_hid_pointing_ready()},
          },
          [this, &buffer](const auto& pair) {
            append_response(buffer,
                            pair.first,
                            pair.second);
          });

      return buffer;
    }

  private:
    // This method is executed in the dispatcher thread.
    void setup_virtual_hid_devices() {
      //
      // Setup virtual_hid_keyboard
      //

      if (virtual_hid_keyboard_enabled_) {
        if (!virtual_hid_keyboard_ready() &&
            !io_service_client_keyboard_) {
          create_virtual_hid_keyboard_client();
        }
      } else {
        io_service_client_keyboard_ = nullptr;
      }

      //
      // Setup virtual_hid_pointing
      //

      if (virtual_hid_pointing_enabled_) {
        if (!virtual_hid_pointing_ready() &&
            !io_service_client_pointing_) {
          create_virtual_hid_pointing_client();
        }
      } else {
        io_service_client_pointing_ = nullptr;
      }
    }

    // This method is executed in the dispatcher thread.
    void create_virtual_hid_keyboard_client() {
      create_virtual_hid_client(
          io_service_client_keyboard_,
          virtual_hid_keyboard_client_generation_id_,
          [this](auto client) {
            client->async_virtual_hid_keyboard_initialize(virtual_hid_keyboard_parameters_);
          },
          [this] {
            return virtual_hid_keyboard_enabled_;
          },
          [this] {
            return virtual_hid_keyboard_ready();
          },
          "recreate io_service_client_keyboard_ since virtual_hid_keyboard is not ready");
    }

    // This method is executed in the dispatcher thread.
    void create_virtual_hid_pointing_client() {
      create_virtual_hid_client(
          io_service_client_pointing_,
          virtual_hid_pointing_client_generation_id_,
          [](auto client) {
            client->async_virtual_hid_pointing_initialize();
          },
          [this] {
            return virtual_hid_pointing_enabled_;
          },
          [this] {
            return virtual_hid_pointing_ready();
          },
          "recreate io_service_client_pointing_ since virtual_hid_pointing is not ready");
    }

    // This method is executed in the dispatcher thread.
    template <typename InitializeClient, typename IsEnabled, typename IsReady>
    void create_virtual_hid_client(std::shared_ptr<io_service_client>& client,
                                   int& client_generation_id,
                                   InitializeClient initialize_client,
                                   IsEnabled is_enabled,
                                   IsReady is_ready,
                                   const char* recreate_log_message) {
      ++client_generation_id;

      auto client_ptr = &client;
      auto client_generation_id_ptr = &client_generation_id;

      client = std::make_shared<io_service_client>(run_loop_thread_,
                                                   log_label_);

      client->opened.connect([client, initialize_client] {
        initialize_client(client);
      });

      client->async_start();

      enqueue_to_dispatcher(
          [this,
           client_ptr,
           client_generation_id_ptr,
           generation_id = client_generation_id,
           is_enabled,
           is_ready,
           recreate_log_message] {
            if (generation_id != *client_generation_id_ptr) {
              return;
            }

            if (is_enabled() &&
                !is_ready()) {
              logger::get_logger()->info(recreate_log_message);

              *client_ptr = nullptr;
              setup_virtual_hid_devices();
            }
          },
          when_now() + std::chrono::milliseconds(5000));
    }

    // This method is executed in the dispatcher thread.
    bool virtual_hid_keyboard_ready() const {
      std::optional<bool> ready;

      if (io_service_client_keyboard_) {
        ready = io_service_client_keyboard_->get_virtual_hid_keyboard_ready();
      }

      return ready.value_or(false);
    }

    // This method is executed in the dispatcher thread.
    bool virtual_hid_pointing_ready() const {
      std::optional<bool> ready;

      if (io_service_client_pointing_) {
        ready = io_service_client_pointing_->get_virtual_hid_pointing_ready();
      }

      return ready.value_or(false);
    }

    void append_response(std::vector<uint8_t>& buffer,
                         pqrs::karabiner::driverkit::virtual_hid_device_service::response response,
                         bool value) const {
      buffer.insert(buffer.end(), {
                                      std::to_underlying(response),
                                      static_cast<uint8_t>(value),
                                  });
    }

    pqrs::not_null_shared_ptr_t<pqrs::cf::run_loop_thread> run_loop_thread_;
    std::string log_label_;

    std::shared_ptr<io_service_client> io_service_client_no_virtual_devices_;
    std::shared_ptr<io_service_client> io_service_client_keyboard_;
    std::shared_ptr<io_service_client> io_service_client_pointing_;
    pqrs::dispatcher::extra::timer ready_timer_;

    // virtual_hid_keyboard
    int virtual_hid_keyboard_client_generation_id_;
    bool virtual_hid_keyboard_enabled_;
    pqrs::karabiner::driverkit::virtual_hid_device_service::virtual_hid_keyboard_parameters virtual_hid_keyboard_parameters_;

    // virtual_hid_pointing
    int virtual_hid_pointing_client_generation_id_;
    bool virtual_hid_pointing_enabled_;
  };

  template <typename GetIoServiceClient>
  void post_report(pqrs::unix_domain_stream::peer_id peer_id,
                   std::shared_ptr<std::vector<uint8_t>> buffer,
                   size_t report_offset,
                   pqrs::karabiner::driverkit::virtual_hid_device_driver::user_client_method user_client_method,
                   const char* report_name,
                   size_t expected_size,
                   GetIoServiceClient get_io_service_client) const {
    if (!dispatcher_thread()) {
      throw std::logic_error(fmt::format("{0} is called in wrong thread", __func__));
    }

    if (!buffer ||
        report_offset > buffer->size()) {
      logger::get_logger()->warn(fmt::format("{0}: buffer range error", __func__));
      return;
    }

    auto report_size = buffer->size() - report_offset;
    if (expected_size != report_size) {
      logger::get_logger()->warn(fmt::format("{0}: buffer size error", __func__));
      return;
    }

    if (auto it = entries_.find(peer_id);
        it != entries_.end()) {
      if (auto client = get_io_service_client(*(it->second))) {
        client->async_post_report(user_client_method,
                                  std::move(buffer),
                                  report_offset,
                                  report_name);
      }
    }
  }

  pqrs::not_null_shared_ptr_t<pqrs::cf::run_loop_thread> run_loop_thread_;
  std::unordered_map<pqrs::unix_domain_stream::peer_id, std::unique_ptr<entry>> entries_;
};
