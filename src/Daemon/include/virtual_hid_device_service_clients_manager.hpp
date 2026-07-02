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
      client_entries_.clear();
    });
  }

  // This method needs to be called in the dispatcher thread.
  void create_client(pqrs::unix_domain_stream::peer_id peer_id) {
    if (!dispatcher_thread()) {
      throw std::logic_error(fmt::format("{0} is called in wrong thread", __func__));
    }

    auto log_label = fmt::format("peer_id:{0}", peer_id);

    if (client_entries_.contains(peer_id)) {
      logger::get_logger()->debug(
          "{0} client already exists",
          log_label);
      return;
    }

    client_entries_[peer_id] = std::make_unique<client_entry>(run_loop_thread_,
                                                              log_label);

    logger::get_logger()->debug("{0} virtual_hid_device_service_clients_manager client is added (size: {1})",
                                log_label,
                                client_entries_.size());
  }

  // This method needs to be called in the dispatcher thread.
  void erase_client(pqrs::unix_domain_stream::peer_id peer_id) {
    if (!dispatcher_thread()) {
      throw std::logic_error(fmt::format("{0} is called in wrong thread", __func__));
    }

    auto log_label = fmt::format("peer_id:{0}", peer_id);

    client_entries_.erase(peer_id);

    logger::get_logger()->debug("{0} virtual_hid_device_service_clients_manager client is removed (size: {1})",
                                log_label,
                                client_entries_.size());
  }

  void initialize_keyboard(pqrs::unix_domain_stream::peer_id peer_id,
                           const pqrs::karabiner::driverkit::virtual_hid_device_service::virtual_hid_keyboard_parameters& parameters) {
    if (!dispatcher_thread()) {
      throw std::logic_error(fmt::format("{0} is called in wrong thread", __func__));
    }

    if (auto it = client_entries_.find(peer_id);
        it != client_entries_.end()) {
      it->second->initialize_keyboard(parameters);
    }
  }

  void terminate_keyboard(pqrs::unix_domain_stream::peer_id peer_id) {
    if (!dispatcher_thread()) {
      throw std::logic_error(fmt::format("{0} is called in wrong thread", __func__));
    }

    if (auto it = client_entries_.find(peer_id);
        it != client_entries_.end()) {
      it->second->terminate_keyboard();
    }
  }

  void initialize_pointing(pqrs::unix_domain_stream::peer_id peer_id) {
    if (!dispatcher_thread()) {
      throw std::logic_error(fmt::format("{0} is called in wrong thread", __func__));
    }

    if (auto it = client_entries_.find(peer_id);
        it != client_entries_.end()) {
      it->second->initialize_pointing();
    }
  }

  void terminate_pointing(pqrs::unix_domain_stream::peer_id peer_id) {
    if (!dispatcher_thread()) {
      throw std::logic_error(fmt::format("{0} is called in wrong thread", __func__));
    }

    if (auto it = client_entries_.find(peer_id);
        it != client_entries_.end()) {
      it->second->terminate_pointing();
    }
  }

  // This method needs to be called in the dispatcher thread.
  void virtual_hid_keyboard_reset(pqrs::unix_domain_stream::peer_id peer_id) const {
    if (!dispatcher_thread()) {
      throw std::logic_error(fmt::format("{0} is called in wrong thread", __func__));
    }

    if (auto it = client_entries_.find(peer_id);
        it != client_entries_.end()) {
      if (auto client = it->second->get_virtual_hid_keyboard_io_service_client()) {
        client->async_virtual_hid_keyboard_reset();
      }
    }
  }

  // This method needs to be called in the dispatcher thread.
  void virtual_hid_pointing_reset(pqrs::unix_domain_stream::peer_id peer_id) const {
    if (!dispatcher_thread()) {
      throw std::logic_error(fmt::format("{0} is called in wrong thread", __func__));
    }

    if (auto it = client_entries_.find(peer_id);
        it != client_entries_.end()) {
      if (auto client = it->second->get_virtual_hid_pointing_io_service_client()) {
        client->async_virtual_hid_pointing_reset();
      }
    }
  }

  // This method needs to be called in the dispatcher thread.
  std::vector<uint8_t> make_response(pqrs::unix_domain_stream::peer_id peer_id) const {
    if (!dispatcher_thread()) {
      throw std::logic_error(fmt::format("{0} is called in wrong thread", __func__));
    }

    if (auto it = client_entries_.find(peer_id);
        it != client_entries_.end()) {
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
                [](const client_entry& client_entry) {
                  return client_entry.get_virtual_hid_keyboard_io_service_client();
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
                [](const client_entry& client_entry) {
                  return client_entry.get_virtual_hid_pointing_io_service_client();
                });
  }

private:
  class client_entry final : public pqrs::dispatcher::extra::dispatcher_client {
  public:
    client_entry(pqrs::not_null_shared_ptr_t<pqrs::cf::run_loop_thread> run_loop_thread,
                 const std::string& log_label)
        : run_loop_thread_(run_loop_thread),
          log_label_(log_label),
          ready_timer_(*this),
          virtual_hid_keyboard_client_generation_id_(0),
          virtual_hid_keyboard_enabled_(false),
          virtual_hid_pointing_client_generation_id_(0),
          virtual_hid_pointing_enabled_(false) {
      no_virtual_devices_io_service_client_ = std::make_shared<io_service_client>(run_loop_thread_,
                                                                                  log_label);
      no_virtual_devices_io_service_client_->async_start();

      ready_timer_.start(
          [this] {
            //
            // Query `ready` state to driver
            //

            if (auto client = virtual_hid_keyboard_io_service_client_) {
              client->async_virtual_hid_keyboard_ready();
            }
            if (auto client = virtual_hid_pointing_io_service_client_) {
              client->async_virtual_hid_pointing_ready();
            }
          },
          std::chrono::milliseconds(1000));
    }

    ~client_entry() {
      detach_from_dispatcher([this] {
        virtual_hid_pointing_io_service_client_ = nullptr;
        virtual_hid_keyboard_io_service_client_ = nullptr;
        no_virtual_devices_io_service_client_ = nullptr;
      });
    }

    std::shared_ptr<io_service_client> get_virtual_hid_keyboard_io_service_client() const {
      return virtual_hid_keyboard_io_service_client_;
    }

    std::shared_ptr<io_service_client> get_virtual_hid_pointing_io_service_client() const {
      return virtual_hid_pointing_io_service_client_;
    }

    //
    // virtual_hid_keyboard_io_service_client_
    //

    void initialize_keyboard(const pqrs::karabiner::driverkit::virtual_hid_device_service::virtual_hid_keyboard_parameters& parameters) {
      enqueue_to_dispatcher([this, parameters] {
        // Destroy virtual_hid_keyboard_io_service_client_ if parameters is changed.
        if (virtual_hid_keyboard_io_service_client_ &&
            virtual_hid_keyboard_parameters_ != parameters) {
          logger::get_logger()->debug("destroy virtual_hid_keyboard_io_service_client_ due to parameter changes");

          virtual_hid_keyboard_io_service_client_ = nullptr;
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
        virtual_hid_keyboard_io_service_client_ = nullptr;
      });
    }

    //
    // virtual_hid_pointing_io_service_client_
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
        virtual_hid_pointing_io_service_client_ = nullptr;
      });
    }

    std::vector<uint8_t> make_response() const {
      std::vector<uint8_t> buffer;
      using response = pqrs::karabiner::driverkit::virtual_hid_device_service::response;

      std::ranges::for_each(
          std::array{
              std::pair{response::driver_activated, no_virtual_devices_io_service_client_->driver_activated()},
              std::pair{response::driver_connected, no_virtual_devices_io_service_client_->driver_connected()},
              std::pair{response::driver_version_mismatched, no_virtual_devices_io_service_client_->driver_version_mismatched()},
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
            !virtual_hid_keyboard_io_service_client_) {
          create_virtual_hid_keyboard_client();
        }
      } else {
        virtual_hid_keyboard_io_service_client_ = nullptr;
      }

      //
      // Setup virtual_hid_pointing
      //

      if (virtual_hid_pointing_enabled_) {
        if (!virtual_hid_pointing_ready() &&
            !virtual_hid_pointing_io_service_client_) {
          create_virtual_hid_pointing_client();
        }
      } else {
        virtual_hid_pointing_io_service_client_ = nullptr;
      }
    }

    // This method is executed in the dispatcher thread.
    void create_virtual_hid_keyboard_client() {
      create_virtual_hid_client(
          virtual_hid_keyboard_io_service_client_,
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
          "recreate virtual_hid_keyboard_io_service_client_ since virtual_hid_keyboard is not ready");
    }

    // This method is executed in the dispatcher thread.
    void create_virtual_hid_pointing_client() {
      create_virtual_hid_client(
          virtual_hid_pointing_io_service_client_,
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
          "recreate virtual_hid_pointing_io_service_client_ since virtual_hid_pointing is not ready");
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

      client = std::make_shared<io_service_client>(run_loop_thread_,
                                                   log_label_);

      client->opened.connect([weak_client = std::weak_ptr<io_service_client>(client),
                              initialize_client] {
        if (auto client = weak_client.lock()) {
          initialize_client(client);
        }
      });

      client->async_start();

      enqueue_to_dispatcher(
          [this,
           client_ptr = &client,
           client_generation_id_ptr = &client_generation_id,
           generation_id = client_generation_id,
           is_enabled,
           is_ready,
           recreate_log_message] {
            if (generation_id != *client_generation_id_ptr) {
              return;
            }

            if (is_enabled() &&
                !is_ready()) {
              logger::get_logger()->debug(recreate_log_message);

              *client_ptr = nullptr;
              setup_virtual_hid_devices();
            }
          },
          when_now() + std::chrono::milliseconds(5000));
    }

    // This method is executed in the dispatcher thread.
    bool virtual_hid_keyboard_ready() const {
      std::optional<bool> ready;

      if (virtual_hid_keyboard_io_service_client_) {
        ready = virtual_hid_keyboard_io_service_client_->get_virtual_hid_keyboard_ready();
      }

      return ready.value_or(false);
    }

    // This method is executed in the dispatcher thread.
    bool virtual_hid_pointing_ready() const {
      std::optional<bool> ready;

      if (virtual_hid_pointing_io_service_client_) {
        ready = virtual_hid_pointing_io_service_client_->get_virtual_hid_pointing_ready();
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

    std::shared_ptr<io_service_client> no_virtual_devices_io_service_client_;
    std::shared_ptr<io_service_client> virtual_hid_keyboard_io_service_client_;
    std::shared_ptr<io_service_client> virtual_hid_pointing_io_service_client_;
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

    if (auto it = client_entries_.find(peer_id);
        it != client_entries_.end()) {
      if (auto client = get_io_service_client(*(it->second))) {
        client->async_post_report(user_client_method,
                                  std::move(buffer),
                                  report_offset,
                                  report_name);
      }
    }
  }

  pqrs::not_null_shared_ptr_t<pqrs::cf::run_loop_thread> run_loop_thread_;
  std::unordered_map<pqrs::unix_domain_stream::peer_id, std::unique_ptr<client_entry>> client_entries_;
};
