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

#include "run_result.hpp"
#include "traits.hpp"

#include <memory>
#include <type_traits>

namespace joque
{

/// Job interface used by task and execution
struct job_iface
{
        /// Returns true in case the job is invalidated
        [[nodiscard]] virtual inval_result is_invalidated() = 0;

        /// Executes one run of the task
        [[nodiscard]] virtual run_result run( const task& ) = 0;

        virtual ~job_iface() = default;
};

/// Implementation of job interface, storing the specific job type that shall be
/// used. Uses job_traits of the job type to run specific behavior. Use the
/// traits for any customization over overriding this.
template < typename T >
struct job : job_iface
{
        T thing;

        job( T thing )
          : thing( std::move( thing ) )
        {
        }

        [[nodiscard]] inval_result is_invalidated() override
        {
                return job_traits< T >::is_invalidated( thing );
        }

        [[nodiscard]] run_result run( const task& t ) override
        {
                return job_traits< T >::run( t, thing );
        }
};

/// Custom unique_ptr wrapper that simplifies syntax of tasks. Allows writing
/// `.job = <expr?>` in instantiation of new task as long as expr representing
/// something convertible to valid `job<T>`.
struct job_ptr : std::unique_ptr< job_iface >
{
        job_ptr() = default;

        template < typename T >
        job_ptr( T&& item )
          : std::unique_ptr< job_iface >(
                new job< std::decay_t< T > >{ std::forward< T >( item ) } )
        {
        }

        job_ptr( const job_ptr& )            = delete;
        job_ptr& operator=( const job_ptr& ) = delete;

        job_ptr( job_ptr&& ) noexcept            = default;
        job_ptr& operator=( job_ptr&& ) noexcept = default;
};

}  // namespace joque
