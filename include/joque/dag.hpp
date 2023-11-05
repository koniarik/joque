#pragma once

#include "joque/task.hpp"

namespace joque
{

/// Node representing all execution-related information for one task.
struct node
{
        /// Full path-name of the task
        std::string name;
        /// Pointer to the task
        const task* t;
        /// Nodes representing tasks that `t` depends on
        std::vector< node* > depends_on{};
        /// Nodes representing tasks that should be run before `t`
        std::vector< node* > run_after{};

        /// Sets to `true` once the task is scheduled for execution
        bool started = false;
        /// Sets to `done` if the task finished it's execution (correctly or incorrectly)
        bool done = false;
        /// Sets to `fail` if the task failed during execution
        bool failed = false;
};

/// DAG used to store data in single execution of tasks
struct dag
{
        std::list< node > nodes;
};

/// Generates a DAG representing a given set of tasks for execution function.
///
/// The DAG becames invalidated after any change to the input task set, and any use of it can result
/// in undefined behavior.
///
/// Function adds any dependencies of task into the `run_after` vector of each node, this way the
/// execution functions does not have to handle dependencies explicitly for order of execution.
///
/// \param ts A set of tasks for which the dag shall be generated
/// \param filter Only nodes, and their dependencies, which match the filter are added to the dag.
///               Uses simple "find exact string match within a string" mechanism.
///
[[nodiscard]] dag generate_dag( const task_set& ts, const std::string& filter = "" );

}  // namespace joque
