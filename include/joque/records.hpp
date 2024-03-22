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

using tp = std::chrono::time_point< std::chrono::system_clock >;

/// Record storing information about a run of one task, produced during single execution once for
/// each task. Note that this valid only as long as the input task set was not modified.
struct run_record
{
        /// Executed task
        std::reference_wrapper< const task > t;

        /// Full name of the task
        std::string name;

        /// Whenever the task was executed or skipped
        bool skipped = false;

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
        f( "skipped", rec.skipped );
        f( "retcode", rec.retcode );
        f( "start", rec.start );
        f( "end", rec.end );
        f( "output", rec.output );
}

/// Record of execution of entire task set. Is valid only as long as the original task set was not
/// modified.
struct exec_record
{
        /// Number of skipped tasks
        std::size_t skipped_count = 0;
        /// Number of failed tasks
        std::size_t failed_count = 0;
        /// Total number of jobs
        std::size_t total_count = 0;
        /// Run record for each finished task in the set.
        std::vector< run_record > runs;
};

std::chrono::seconds runtime_sum( const exec_record& erec );

void map( std::convertible_to< exec_record > auto& rec, auto&& f )
{
        f( "skipped_count", rec.skipped_count );
        f( "failed_count", rec.failed_count );
        f( "total_count", rec.total_count );
        f( "runs", rec.runs );
}

#ifdef NLOHMANN_JSON_NAMESPACE_VERSION


inline void to_json( nlohmann::json& j, const output_chunk& rec )
{
        map( rec, [&]( const char* key, auto& val ) {
                j[key] = val;
        } );
}

inline void to_json( nlohmann::json& j, const run_record& rec )
{
        map( rec, [&]< typename T >( const char* key, T& val ) {
                if constexpr ( std::same_as< T, const tp > )
                        j[key] = std::format( "{}", val );
                else if constexpr ( !std::same_as< T, const std::reference_wrapper< const task > > )
                        j[key] = val;
        } );
}

inline void to_json( nlohmann::json& j, const exec_record& rec )
{
        map( rec, [&]( const char* key, auto& val ) {
                j[key] = val;
        } );
}

#endif

}  // namespace joque
