#pragma once

#include "joque/run_result.hpp"
#include "joque/task.hpp"

#include <chrono>
#include <cstddef>
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

/// Record of execution of entire task set. Is valid only as long as the original task set was not
/// modified.
struct exec_record
{
        /// Number of skipped tasks
        std::size_t skipped_count = 0;
        /// Number of failed tasks
        std::size_t failed_count = 0;
        /// Run record for each finished task in the set.
        std::vector< run_record > runs;
};

}  // namespace joque
