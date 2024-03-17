#pragma once

#include "joque/run_result.hpp"


namespace joque
{

struct task;

/// Default job traits for all types. Assumes that the type is callable with signature
/// `run_result(const task*)`.
template < typename T >
struct job_traits
{
        [[nodiscard]] static bool is_invalidated( const T& )
        {
                return true;
        }

        [[nodiscard]] static run_result run( const task& t, const T& f )
        {
                return f( t );
        }
};

}  // namespace joque
