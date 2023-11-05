#pragma once

#include "joque/process.hpp"
#include "joque/traits.hpp"

#include <filesystem>
#include <functional>
#include <list>
#include <string>
#include <vector>

namespace joque
{

struct job_iface
{
        virtual [[nodiscard]] bool       is_invalidated()   = 0;
        virtual [[nodiscard]] run_result run( const task* ) = 0;
        virtual ~job_iface()                                = default;
};

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

        [[nodiscard]] run_result run( const task* t ) override
        {
                return job_traits< T >::run( t, thing );
        }
};

struct job_ptr : std::unique_ptr< job_iface >
{
        job_ptr() = default;

        template < typename T >
        job_ptr( T&& item )
          : std::unique_ptr< job_iface >(
                new job< std::decay_t< T > >{ std::forward< T >( item ) } )
        {
        }

        job_ptr( const job_ptr& )                     = delete;
        job_ptr& operator=( const job_ptr& ) noexcept = delete;
        job_ptr( job_ptr&& )                          = default;
        job_ptr& operator=( job_ptr&& ) noexcept      = default;
};

}  // namespace joque
