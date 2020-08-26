#pragma once

// (C) Copyright Takayama Fumihiko 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See http://www.boost.org/LICENSE_1_0.txt)

#include <string_view>

namespace pqrs {
namespace karabiner {
namespace driverkit {
namespace virtual_hid_device_service {
namespace constants {
constexpr std::string_view server_socket_file_path = "/Library/Application Support/org.pqrs/tmp/rootonly/karabiner_driverkit_virtual_hid_device_server";
} // namespace constants
} // namespace virtual_hid_device_service
} // namespace driverkit
} // namespace karabiner
} // namespace pqrs
