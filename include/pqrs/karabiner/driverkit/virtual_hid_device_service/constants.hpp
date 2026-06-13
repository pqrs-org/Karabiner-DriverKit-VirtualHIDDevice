#pragma once

// (C) Copyright Takayama Fumihiko 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See https://www.boost.org/LICENSE_1_0.txt)

#include <cstddef>
#include <filesystem>
#include <string_view>

namespace pqrs::karabiner::driverkit::virtual_hid_device_service::constants {
inline std::filesystem::path get_rootonly_directory() {
  return "/Library/Application Support/org.pqrs/tmp/rootonly";
}

inline std::filesystem::path get_server_socket_file_path() {
  // Note:
  // The socket file path length must be <= 103 because sizeof(sockaddr_un.sun_path) == 104.
  // "/Library/Application Support/org.pqrs/tmp/rootonly/karabiner_virtual_hid_device_service.sock" length is 99.

  return get_rootonly_directory() / "karabiner_virtual_hid_device_service.sock";
}

constexpr std::size_t unix_domain_stream_max_message_size = 1024;
} // namespace pqrs::karabiner::driverkit::virtual_hid_device_service::constants
