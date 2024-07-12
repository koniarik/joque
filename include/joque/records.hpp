#pragma once

#include "run_result.hpp"
#include "task.hpp"

#include <chrono>
#include <cstddef>
#include <format>
#include <functional>
#include <list>
#include <map>
#include <string>
#include <string_view>
#include <vector>

namespace joque
{

enum class run_status : uint8_t
{
        OK,
        SKIP,
        DEPF,
        FAIL,
};

std::string_view to_sv( const run_status& s );

using tp = std::chrono::time_point< std::chrono::system_clock >;

/// Record storing information about a run of one task, produced during single
/// execution once for each task. Note that this valid only as long as the input
/// task set was not modified.
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

/// Record of execution of entire task set. Is valid only as long as the
/// original task set was not modified.
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

void to_json( nlohmann::json& j, const output_chunk& rec );

void to_json( nlohmann::json& j, const run_record& rec );

void to_json( nlohmann::json& j, const exec_record& rec );

#endif

}  // namespace joque
