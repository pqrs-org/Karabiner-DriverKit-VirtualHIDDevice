#pragma once

// (C) Copyright Takayama Fumihiko 2026.
// Distributed under the Boost Software License, Version 1.0.
// (See https://www.boost.org/LICENSE_1_0.txt)

#ifdef ASIO_STANDALONE
#include <asio.hpp>
#else
#define ASIO_STANDALONE
#include <asio.hpp>
#undef ASIO_STANDALONE
#endif

#include <filesystem>

namespace pqrs::unix_domain_stream::impl::asio_helper {

// Convert endpoint construction errors into the same error-code path as I/O errors.
inline asio::local::stream_protocol::endpoint make_endpoint(const std::filesystem::path& path,
                                                            asio::error_code& error_code) {
  try {
    auto endpoint = asio::local::stream_protocol::endpoint(path);
    error_code.clear();
    return endpoint;
  } catch (const asio::system_error& error) {
    error_code = error.code();
    return {};
  }
}

namespace time_point {
[[nodiscard]] inline asio::steady_timer::time_point now() noexcept {
  return asio::steady_timer::clock_type::now();
}

[[nodiscard]] inline asio::steady_timer::time_point pos_infin() noexcept {
  return asio::steady_timer::time_point::max();
}
} // namespace time_point

} // namespace pqrs::unix_domain_stream::impl::asio_helper
