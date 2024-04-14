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
        [[nodiscard]] static inval_result is_invalidated( const T& )
        {
                return { .invalidated = true, .log = { "always invalidated" } };
        }

        [[nodiscard]] static run_result run( const task& t, T& f )
        {
                try {
                        return f( t );
                }
                catch ( std::exception& e ) {
                        run_result res;
                        insert_err( res, "Uncaught exception:" );
                        insert_err( res, e.what() );
                        return res;
                }
        }
};

}  // namespace joque
