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
                promise_type( dag_node& n, Args&&... )
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

                dag_node&                   n;
                std::optional< run_record > value;
                std::exception_ptr          excep;
        };

        run_coro( dag_node& n, std::coroutine_handle< promise_type > h )
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
                if ( !h_ )
                        return nullptr;
                if ( h_.promise().excep )
                        std::rethrow_exception( h_.promise().excep );
                std::optional< run_record >& val = h_.promise().value;
                if ( val.has_value() )
                        return &*val;
                return nullptr;
        }

        void tick()
        {
                if ( h_ )
                        h_();
        }

        run_record* run()
        {
                while ( !done() )
                        tick();

                return result();
        }

        [[nodiscard]] dag_node& get_node()
        {
                return node_;
        }

        [[nodiscard]] const dag_node& get_node() const
        {
                return node_;
        }

        ~run_coro()
        {
                if ( h_ )
                        h_.destroy();
        }

private:
        dag_node&                             node_;
        std::coroutine_handle< promise_type > h_;
};

}  // namespace joque
