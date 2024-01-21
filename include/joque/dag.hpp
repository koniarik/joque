#pragma once

#include "joque/list_node.hpp"
#include "joque/task.hpp"

#include <functional>
#include <list>

namespace joque
{

struct dag_node;
struct dag_edge;
struct dag_edge_accessor;

using dag_edge_lnode = list_node< dag_edge, dag_edge_accessor >;

struct dag_edge
{
        bool           is_dependency = false;
        dag_node*      target        = nullptr;
        dag_edge_lnode lnode;
};

using dag_edge_list = list< dag_edge_lnode >;

struct dag_edge_accessor
{
        static auto& get( auto& n )
        {
                return n.lnode;
        }
};

/// Node representing all execution-related information for one task.
struct dag_node
{
        /// Full path-name of the task
        std::string name;
        /// Reference to the task
        std::reference_wrapper< const task > t;

        /// Nodes representing tasks that should be run before `t`
        dag_edge_list runs_after{};

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
        std::list< dag_node > nodes;
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
