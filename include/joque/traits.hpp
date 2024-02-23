#pragma once

#include <list>
#include <string>
#include <tuple>

namespace joque
{

struct task_iface;

struct output_chunk
{
        enum type_e
        {
                ERROR,
                STANDARD
        };
        type_e      type;
        std::string data;
};

/// Result of single traits run call. Information is stored in run record.
struct run_result
{
        /// Return code of the run, 0 implies success.
        int retcode;

        std::list< output_chunk > output;

        [[nodiscard]] operator std::tuple< int&, std::list< output_chunk >& >() &&
        {
                return { retcode, output };
        }
};

inline void record_output( run_result& res, output_chunk::type_e type, std::string_view data )
{
        res.output.emplace_back( type, std::string( data ) );
}

/// Default job traits for all types. Assumes that the type is callable with signature
/// `run_result(const task*)`.
template < typename T >
struct job_traits
{
        [[nodiscard]] static bool is_invalidated( const T& )
        {
                return true;
        }

        [[nodiscard]] static run_result run( const task_iface& t, const T& f )
        {
                return f( t );
        }
};

}  // namespace joque
