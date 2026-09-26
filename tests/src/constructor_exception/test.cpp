#include "io_service_client.hpp"
#include "virtual_hid_device_service_clients_manager.hpp"
#include <cstdio>
#include <cstdlib>
#include <future>
#include <new>

namespace {
// Fail one allocation on the constructing thread only. Cleanup and background
// dispatcher/run-loop work must remain able to allocate after the injected failure.
thread_local long allocations_before_failure = -1;
} // namespace

void* operator new(std::size_t size) {
  if (allocations_before_failure >= 0 && allocations_before_failure-- == 0) {
    throw std::bad_alloc();
  }
  if (auto p = std::malloc(size ? size : 1)) {
    return p;
  }
  throw std::bad_alloc();
}

void* operator new[](std::size_t size) {
  return ::operator new(size);
}

void operator delete(void* p) noexcept {
  std::free(p);
}

void operator delete[](void* p) noexcept {
  ::operator delete(p);
}

namespace {
template <typename Factory>
bool check_constructor(const char* name, Factory factory) {
  int failures = 0;
  for (long index = 0; index < 512; ++index) {
    allocations_before_failure = index;
    try {
      auto object = factory();
      // Do not inject failures into the normal destructor after construction.
      allocations_before_failure = -1;
      if (failures == 0) {
        return false;
      }
      std::printf("%s: recovered from %d allocation failures\n", name, failures);
      return true;
    } catch (const std::bad_alloc&) {
      allocations_before_failure = -1;
      ++failures;
    }
  }
  allocations_before_failure = -1;
  std::fprintf(stderr, "%s: never completed construction\n", name);
  return false;
}
} // namespace

int main() {
  pqrs::dispatcher::extra::initialize_shared_dispatcher();
  auto dispatcher = pqrs::dispatcher::extra::get_shared_dispatcher();
  auto run_loop_thread = std::make_shared<pqrs::cf::run_loop_thread>();
  const std::string log_label(128, 'x');
  // Initialize the logger before injecting allocation failures.
  static_cast<void>(logger::get_logger());

  bool passed = true;
  passed &= check_constructor("public client", [] {
    return std::make_unique<pqrs::karabiner::driverkit::virtual_hid_device_service::client>();
  });
  passed &= check_constructor("io_service_client", [&] {
    return std::make_unique<io_service_client>(dispatcher, run_loop_thread, log_label);
  });
  passed &= check_constructor("clients_manager", [&] {
    return std::make_unique<virtual_hid_device_service_clients_manager>(dispatcher, run_loop_thread);
  });

  // Construct and destroy entries within one dispatcher task. Their queued
  // IOKit startup is canceled before it runs, so no driver connection is made.
  std::promise<bool> result;
  auto future = result.get_future();
  pqrs::dispatcher::extra::dispatcher_client task(dispatcher);
  task.enqueue_to_dispatcher([&] {
    result.set_value(check_constructor("client_entry", [&] {
      auto manager = std::make_unique<virtual_hid_device_service_clients_manager>(dispatcher, run_loop_thread);
      manager->create_client(pqrs::unix_domain_stream::peer_id{1});
      return manager;
    }));
  });
  passed &= future.get();
  task.detach_from_dispatcher();

  run_loop_thread->terminate();
  pqrs::dispatcher::extra::terminate_shared_dispatcher();
  return passed ? 0 : 1;
}
