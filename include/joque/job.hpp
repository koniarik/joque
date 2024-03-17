#pragma once

#include "joque/run_result.hpp"
#include "joque/traits.hpp"

#include <memory>
#include <type_traits>

namespace joque
{

/// Job interface used by task and execution
struct job_iface
{
        /// Returns true in case the job is invalidated
        [[nodiscard]] virtual bool is_invalidated() = 0;

        /// Executes one run of the task, returns `run_result` with properly filled information.
        [[nodiscard]] virtual run_result run( const task& ) = 0;

        virtual ~job_iface() = default;
};

/// Implementation of job interface, storing the specific job type that shall be used. Uses
/// job_traits of the job type to run specific behavior. Use the traits for any customization over
/// overriding this.
template < typename T >
struct job : job_iface
{
        T thing;

        job( T thing )
          : thing( std::move( thing ) )
        {
        }

        [[nodiscard]] bool is_invalidated() override
        {
                return job_traits< T >::is_invalidated( thing );
        }

        [[nodiscard]] run_result run( const task& t ) override
        {
                return job_traits< T >::run( t, thing );
        }
};

/// Custom unique_ptr wrapper that simplifies syntax of tasks. Allows writing `.job = <expr?>` in
/// instantiation of new task as long as expr representings something convertible to valid `job<T>`.
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
