#pragma once

#include "joque/dag.hpp"
#include "joque/records.hpp"

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
                promise_type( node& n, Args&&... )
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

        [[nodiscard]] bool done() const
        {
                return h_.done();
        }

        [[nodiscard]] run_record* result()
        {
                if ( !h_ ) {
                        return nullptr;
                }
                if ( h_.promise().excep ) {
                        std::rethrow_exception( h_.promise().excep );
                }
                std::optional< run_record >& val = h_.promise().value;
                if ( val.has_value() ) {
                        return &*val;
                }
                return nullptr;
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

        [[nodiscard]] node& get_node()
        {
                return node_;
        }

        [[nodiscard]] const node& get_node() const
        {
                return node_;
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
