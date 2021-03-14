#pragma once

// (C) Copyright Takayama Fumihiko 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See https://www.boost.org/LICENSE_1_0.txt)

#include <string_view>

namespace pqrs {
namespace karabiner {
namespace driverkit {
namespace virtual_hid_device_service {
namespace constants {
constexpr std::string_view rootonly_directory = "/Library/Application Support/org.pqrs/tmp/rootonly";
// Note:
// The socket file path length must be <= 103 because sizeof(sockaddr_un.sun_path) == 104.
// So we use the shorten name virtual_hid_device_service_server => vhidd_server.
constexpr std::string_view server_socket_directory_path = "/Library/Application Support/org.pqrs/tmp/rootonly/vhidd_server";
constexpr std::size_t local_datagram_buffer_size = 1024;
} // namespace constants
} // namespace virtual_hid_device_service
} // namespace driverkit
} // namespace karabiner
} // namespace pqrs
