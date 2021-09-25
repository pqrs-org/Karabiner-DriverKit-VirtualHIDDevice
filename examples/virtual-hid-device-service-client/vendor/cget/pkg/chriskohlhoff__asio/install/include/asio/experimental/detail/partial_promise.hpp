//
// experimental/detail/partial_promise.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2021 Klemens D. Morgenstern
//                    (klemens dot morgenstern at gmx dot net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_EXPERIMENTAL_DETAIL_PARTIAL_PROMISE_HPP
#define ASIO_EXPERIMENTAL_DETAIL_PARTIAL_PROMISE_HPP

#include "asio/detail/config.hpp"
#include "asio/experimental/detail/coro_traits.hpp"
#include "asio/awaitable.hpp"
#include <iostream>

#if defined(ASIO_HAS_STD_COROUTINE)
# include <coroutine>
#else // defined(ASIO_HAS_STD_COROUTINE)
# include <experimental/coroutine>
#endif // defined(ASIO_HAS_STD_COROUTINE)

namespace asio {
namespace experimental {
namespace detail {

#if defined(ASIO_HAS_STD_COROUTINE)

using std::coroutine_handle;
using std::coroutine_traits;
using std::suspend_never;
using std::suspend_always;
using std::noop_coroutine;

#else // defined(ASIO_HAS_STD_COROUTINE)

using std::experimental::coroutine_handle;
using std::experimental::coroutine_traits;
using std::experimental::suspend_never;
using std::experimental::suspend_always;
using std::experimental::noop_coroutine;

#endif // defined(ASIO_HAS_STD_COROUTINE)

struct partial_promise
{
  auto initial_suspend() noexcept
  {
    return asio::detail::suspend_always{};
  }

  auto final_suspend() noexcept
  {
    struct awaitable_t
    {
      partial_promise *p;

      constexpr bool await_ready() noexcept { return true; }

      auto await_suspend(asio::detail::coroutine_handle<>) noexcept
      {
        p->get_return_object().destroy();
      }

      constexpr void await_resume() noexcept {}
    };

    return awaitable_t{this};
  }

  void return_void() {}

  coroutine_handle<partial_promise> get_return_object()
  {
    return coroutine_handle<partial_promise>::from_promise(*this);
  }

  void unhandled_exception()
  {
    assert(false);
  }
};

} // namespace detail
} // namespace experimental
} // namespace asio

#if defined(ASIO_HAS_STD_COROUTINE)

namespace std {

template<typename ... Args>
struct coroutine_traits<
    std::experimental::coroutine_handle<coro::partial_promise>, Args...>
{
  using promise_type = coro::partial_promise;
};

} // namespace std

#else // defined(ASIO_HAS_STD_COROUTINE)

namespace std { namespace experimental {

template<typename... Args>
struct coroutine_traits<
    coroutine_handle<asio::experimental::detail::partial_promise>,
    Args...>
{
  using promise_type = asio::experimental::detail::partial_promise;
};

}} // namespace std::experimental

#endif // defined(ASIO_HAS_STD_COROUTINE)

namespace asio {
namespace experimental {
namespace detail {

template<typename CompletionToken>
auto post_coroutine(CompletionToken token) noexcept
  -> coroutine_handle<partial_promise>
{
  post(std::move(token));
  co_return;
}

template<execution::executor Executor, typename CompletionToken>
auto post_coroutine(Executor exec, CompletionToken token) noexcept
  -> coroutine_handle<partial_promise>
{
  post(exec, std::move(token));
  co_return;
}

template<execution_context Context, typename CompletionToken>
auto post_coroutine(Context &ctx, CompletionToken token) noexcept
  -> coroutine_handle<partial_promise>
{
  post(ctx, std::move(token));
  co_return;
}

template<typename CompletionToken>
auto dispatch_coroutine(CompletionToken token) noexcept
  -> coroutine_handle<partial_promise>
{
  dispatch(std::move(token));
  co_return;
}

template<execution::executor Executor, typename CompletionToken>
auto dispatch_coroutine(Executor exec, CompletionToken token) noexcept
  -> coroutine_handle<partial_promise>
{
  dispatch(exec, std::move(token));
  co_return;
}

template<execution_context Context, typename CompletionToken>
auto dispatch_coroutine(Context &ctx, CompletionToken token) noexcept
  -> coroutine_handle<partial_promise>
{
  dispatch(ctx, std::move(token));
  co_return;
}

} // namespace detail
} // namespace experimental
} // namespace asio

#endif // ASIO_EXPERIMENTAL_DETAIL_PARTIAL_PROMISE_HPP
