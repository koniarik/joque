#pragma once

#include <string>
#include <tuple>
#include <vector>

namespace joque
{

struct task;

/// Result of single traits run call. Information is stored in run record.
struct run_result
{
        /// Return code of the run, 0 implies success.
        int retcode;

        /// Standard output of the run
        std::string std_out;

        /// Error output of the run
        std::string std_err;

        [[nodiscard]] operator std::tuple< int&, std::string&, std::string& >() &&
        {
                return { retcode, std_out, std_err };
        }
};

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
