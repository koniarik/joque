#pragma once

#include "joque/dag.hpp"
#include "joque/exec_coro.hpp"
#include "joque/exec_visitor.hpp"
#include "joque/print_exec_visitor.hpp"
#include "joque/task.hpp"

#include <string>
#include <thread>

namespace joque
{

/// Runs execution of tasks within one taskset.
///
/// This returns coroutine handling the execution process. Withing it lives a thread pool that
/// pararelly executes all tasks in the task_set. Tasks are limited only to those that pass the
/// filter, or their dependencies.
///
/// In case any task has defined dependencies on resource, only one task at a time would be executed
/// for each resource.
///
/// During the execution, the input task set shall not be modified and any modification might result
/// in undefined behavior. After the execution, the produced records structure is valid only for as
/// long as the input task_set was not modified.
///
/// In case 1...n threads are requested, the tasks are executed inside those worker threads and
/// control thread is not used for execution. In case the thread count is 0, all jobs are executed
/// within the control thread itself.
///
/// \param ts Input task set to process
/// \param thread_count Number of threads to use
/// \param filter String filtering out tasks that either match it, or are dependencies of matched
/// tasks
/// \param vis Visitor called by the execution on various events
///
[[nodiscard]] exec_coro exec(
    const task_set&    ts,
    unsigned           thread_count = std::thread::hardware_concurrency(),
    const std::string& filter       = "",
    exec_visitor&      vis          = PRINT_VISITOR );

/// Overload of `exec` which uses dag as an input instead of task set. It's not recommended to use
/// it, but it exists in case users want to customize the dag.
[[nodiscard]] exec_coro exec(
    dag           g,
    unsigned      thread_count = std::thread::hardware_concurrency(),
    exec_visitor& vis          = PRINT_VISITOR );

}  // namespace joque
