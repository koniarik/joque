#pragma once

#include "joque/run_result.hpp"
#include "joque/task.hpp"

#include <chrono>
#include <cstddef>
#include <format>
#include <functional>
#include <list>
#include <string>
#include <vector>

namespace joque
{

enum class run_status
{
        OK,
        SKIP,
        DEPF,
        FAIL,
};

inline std::string_view to_sv( const run_status& s )
{
        switch ( s ) {
        case joque::run_status::OK:
                return "OK";
        case joque::run_status::SKIP:
                return "SKIP";
        case joque::run_status::DEPF:
                return "DEPF";
        case joque::run_status::FAIL:
                return "FAIL";
        }
        return "";
}

using tp = std::chrono::time_point< std::chrono::system_clock >;

/// Record storing information about a run of one task, produced during single execution once for
/// each task. Note that this valid only as long as the input task set was not modified.
struct run_record
{
        /// Executed task
        std::reference_wrapper< const task > t;

        /// Full name of the task
        std::string name;

        run_status status = run_status::OK;

        /// Return code of the execution. Assumes that 0 means success.
        int retcode = 0;

        /// Start time of the execution
        tp start = std::chrono::system_clock::now();

        /// End time of the execution
        tp end = std::chrono::system_clock::now();

        std::list< output_chunk > output;
};

void map( std::convertible_to< run_record > auto& rec, auto&& f )
{
        f( "t", rec.t );
        f( "name", rec.name );
        f( "status", rec.status );
        f( "retcode", rec.retcode );
        f( "start", rec.start );
        f( "end", rec.end );
        f( "output", rec.output );
}

/// Record of execution of entire task set. Is valid only as long as the original task set was not
/// modified.
struct exec_record
{
        std::map< run_status, std::size_t > stats{
            { run_status::OK, 0 },
            { run_status::SKIP, 0 },
            { run_status::DEPF, 0 },
            { run_status::FAIL, 0 },
        };
        /// Total number of jobs
        std::size_t total_count = 0;
        /// Run record for each finished task in the set.
        std::vector< run_record > runs;
};

std::chrono::seconds runtime_sum( const exec_record& erec );

void map( std::convertible_to< exec_record > auto& rec, auto&& f )
{
        for ( auto&& [k, v] : rec.stats )
                f( to_sv( k ), v );
        f( "total_count", rec.total_count );
        f( "runs", rec.runs );
}

#ifdef NLOHMANN_JSON_NAMESPACE_VERSION


inline void to_json( nlohmann::json& j, const output_chunk& rec )
{
        map( rec, [&]( const std::string_view key, auto& val ) {
                j[key] = val;
        } );
}

inline void to_json( nlohmann::json& j, const run_record& rec )
{
        map( rec, [&]< typename T >( const std::string_view key, T& val ) {
                if constexpr ( std::same_as< T, const tp > )
                        j[key] = std::format( "{}", val );
                else if constexpr ( !std::same_as< T, const std::reference_wrapper< const task > > )
                        j[key] = val;
        } );
}

inline void to_json( nlohmann::json& j, const exec_record& rec )
{
        map( rec, [&]( const std::string_view key, auto& val ) {
                j[key] = val;
        } );
}

#endif

}  // namespace joque
