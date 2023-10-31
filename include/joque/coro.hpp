#pragma once

#include <coroutine>
#include <exception>
#include <optional>
#include <utility>

namespace joque
{

template < typename T >
class task_coro
{
public:
        struct promise_type
        {
                [[nodiscard]] task_coro get_return_object()
                {
                        return task_coro{
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

                std::optional< T > value;
                std::exception_ptr excep;
        };

        task_coro( std::coroutine_handle< promise_type > h )
          : h_( h )
        {
        }

        task_coro( task_coro&& other ) noexcept
        {
                std::swap( h_, other.h_ );
        }

        task_coro& operator=( task_coro&& other ) noexcept
        {
                std::swap( h_, other.h_ );
                return *this;
        }

        bool done() const
        {
                return h_.done();
        }

        T* result()
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

        T* run()
        {
                while ( !done() ) {
                        tick();
                }

                return result();
        }

        ~task_coro()
        {
                if ( h_ ) {
                        h_.destroy();
                }
        }

private:
        std::coroutine_handle< promise_type > h_;
};

}  // namespace joque
