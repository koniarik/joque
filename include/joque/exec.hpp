#pragma once

#include "joque/task.hpp"

#include <coroutine>
#include <optional>

namespace joque
{

using tp = std::chrono::time_point< std::chrono::system_clock >;

struct run_record
{
        const task* t;
        std::string name;
        bool        skipped = false;
        int         retcode = 0;
        tp          start   = std::chrono::system_clock::now();
        tp          end     = std::chrono::system_clock::now();
        std::string std_out;
        std::string std_err;
};

struct exec_record
{
        std::size_t               skipped_count = 0;
        std::size_t               failed_count  = 0;
        std::vector< run_record > runs;
};

class exec_coro
{
public:
        struct promise_type
        {

                [[nodiscard]] exec_coro get_return_object()
                {
                        return exec_coro{
                            std::coroutine_handle< promise_type >::from_promise( *this ) };
                }

                [[nodiscard]] std::suspend_always initial_suspend() const
                {
                        return {};
                }

                [[nodiscard]] std::suspend_always final_suspend() const noexcept
                {
                        return {};
                }

                void unhandled_exception()
                {
                        excep = std::current_exception();
                }

                template < typename U >
                void return_value( U&& val )
                {
                        value = std::forward< U >( val );
                }

                std::optional< exec_record > value;
                std::exception_ptr           excep;
        };

        exec_coro( std::coroutine_handle< promise_type > h )
          : h_( h )
        {
        }

        exec_coro( exec_coro&& other ) noexcept
        {
                std::swap( h_, other.h_ );
        }

        exec_coro& operator=( exec_coro&& other ) noexcept
        {
                std::swap( h_, other.h_ );
                return *this;
        }

        bool done() const
        {
                return h_.done();
        }

        exec_record* result()
        {
                if ( !h_ ) {
                        return nullptr;
                }
                if ( !h_.promise().value ) {
                        return nullptr;
                }
                if ( h_.promise().excep ) {
                        std::rethrow_exception( h_.promise().excep );
                }
                return &*h_.promise().value;
        }

        void tick()
        {
                if ( h_ ) {
                        h_();
                }
        }

        exec_record* run()
        {
                while ( !done() ) {
                        tick();
                }

                return result();
        }

        ~exec_coro()
        {
                if ( h_ ) {
                        h_.destroy();
                }
        }

private:
        std::coroutine_handle< promise_type > h_;
};

exec_coro exec( const task_set& ts, unsigned thread_count = 0, const std::string& filter = "" );

}  // namespace joque
