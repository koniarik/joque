#pragma once

#include <coroutine>
#include <exception>
#include <optional>
#include <utility>

namespace joque
{

class run_coro
{
public:
        struct promise_type
        {
                template < typename... Args >
                promise_type( node& n, Args&&... args )
                  : n( n )
                {
                }

                [[nodiscard]] run_coro get_return_object()
                {
                        return run_coro{
                            n, std::coroutine_handle< promise_type >::from_promise( *this ) };
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

                node&                       n;
                std::optional< run_record > value;
                std::exception_ptr          excep;
        };

        run_coro( node& n, std::coroutine_handle< promise_type > h )
          : node_( n )
          , h_( h )
        {
        }

        run_coro( run_coro&& other ) noexcept
          : node_( other.node_ )
        {
                std::swap( h_, other.h_ );
        }

        run_coro& operator=( run_coro&& other ) noexcept
        {
                std::swap( h_, other.h_ );
                return *this;
        }

        bool done() const
        {
                return h_.done();
        }

        run_record* result()
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

        run_record* run()
        {
                while ( !done() ) {
                        tick();
                }

                return result();
        }

        ~run_coro()
        {
                if ( h_ ) {
                        h_.destroy();
                }
        }

private:
        node&                                 node_;
        std::coroutine_handle< promise_type > h_;
};

}  // namespace joque
