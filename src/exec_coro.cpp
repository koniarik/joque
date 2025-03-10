/// MIT License
///
/// Copyright (c) 2025 Jan Veverak Koniarik
///
/// Permission is hereby granted, free of charge, to any person obtaining a copy
/// of this software and associated documentation files (the "Software"), to deal
/// in the Software without restriction, including without limitation the rights
/// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
/// copies of the Software, and to permit persons to whom the Software is
/// furnished to do so, subject to the following conditions:
///
/// The above copyright notice and this permission notice shall be included in all
/// copies or substantial portions of the Software.
///
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
/// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
/// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
/// SOFTWARE.
#include "joque/exec_coro.hpp"

#include "joque/records.hpp"

#include <chrono>
#include <coroutine>
#include <exception>
#include <optional>
#include <thread>
#include <utility>

namespace joque
{
[[nodiscard]] exec_coro exec_coro::promise_type::get_return_object()
{
        return exec_coro{ std::coroutine_handle< promise_type >::from_promise( *this ) };
}

[[nodiscard]] std::suspend_always exec_coro::promise_type::initial_suspend() const
{
        return {};
}

[[nodiscard]] std::suspend_always exec_coro::promise_type::final_suspend() const noexcept
{
        return {};
}

void exec_coro::promise_type::unhandled_exception()
{
        excep = std::current_exception();
}

void exec_coro::promise_type::return_value( exec_record rec )
{
        value = std::move( rec );
}

exec_coro::exec_coro( std::coroutine_handle< promise_type > h )
  : h_( h )
{
}

exec_coro::exec_coro( exec_coro&& other ) noexcept
{
        std::swap( h_, other.h_ );
}

exec_coro& exec_coro::operator=( exec_coro&& other ) noexcept
{
        std::swap( h_, other.h_ );
        return *this;
}

[[nodiscard]] bool exec_coro::done() const
{
        return h_.done();
}

[[nodiscard]] std::optional< exec_record > exec_coro::result()
{
        if ( !h_ )
                return std::nullopt;
        if ( h_.promise().excep )
                std::rethrow_exception( h_.promise().excep );
        const std::optional< exec_record >& val = h_.promise().value;
        return val;
}

void exec_coro::tick()
{
        if ( h_ )
                h_();
}

std::optional< exec_record > exec_coro::run( std::chrono::milliseconds period )
{
        auto next = std::chrono::steady_clock::now();
        while ( !done() ) {
                std::this_thread::sleep_until( next );
                tick();
                next += period;
        }

        return result();
}

exec_coro::~exec_coro()
{
        if ( h_ )
                h_.destroy();
}

}  // namespace joque
