#include "joque/exec_coro.hpp"

#include <thread>

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

[[nodiscard]] exec_record* exec_coro::result()
{
        if ( !h_ )
                return nullptr;
        if ( h_.promise().excep )
                std::rethrow_exception( h_.promise().excep );
        std::optional< exec_record >& val = h_.promise().value;
        if ( val )
                return &*val;
        return nullptr;
}

void exec_coro::tick()
{
        if ( h_ )
                h_();
}

exec_record* exec_coro::run( std::chrono::milliseconds period )
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
