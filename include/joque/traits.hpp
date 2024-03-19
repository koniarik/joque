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

        [[nodiscard]] static run_result run( const task& t, T& f )
        {
                try {
                        return f( t );
                }
                catch ( std::exception& e ) {
                        run_result res;
                        record_output( res, output_chunk::ERROR, "Uncaught exception:" );
                        record_output( res, output_chunk::ERROR, e.what() );
                        return res;
                }
        }
};

}  // namespace joque
