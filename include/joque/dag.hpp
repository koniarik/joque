#pragma once

#include "joque/bits/list.hpp"
#include "joque/task.hpp"

#include <functional>
#include <list>

namespace joque
{

struct dag_lheader_accessor
{
        static auto& get( auto& n )
        {
                return n.lheader;
        }
};

struct dag_node;
struct dag_edge;

using dag_edge_lheader = bits::list_header< dag_edge, dag_lheader_accessor >;

struct dag_edge
{
        bool             is_dependency = false;
        dag_node*        target        = nullptr;
        dag_edge_lheader lheader;
};

using dag_edge_list = bits::list< dag_edge_lheader, true >;

using dag_node_lheader = bits::list_header< dag_node, dag_lheader_accessor >;

/// Node representing all execution-related information for one task.
struct dag_node
{
        /// Full path-name of the task
        std::string name;
        /// Reference to the task
        const task* t = nullptr;

        /// Nodes representing tasks that should be run before `t`
        dag_edge_list runs_after{};

        /// Sets to `true` once the task is scheduled for execution
        bool started = false;
        /// Sets to `done` if the task finished it's execution (correctly or incorrectly)
        bool done = false;
        /// Sets to `fail` if the task failed during execution
        bool failed = false;

        dag_node_lheader lheader;
};

using dag_node_list = bits::list< dag_node_lheader, true >;

/// DAG used to store data in single execution of tasks
class dag
{
public:
        dag() = default;

        auto begin()
        {
                return nodes_.begin();
        }
        auto begin() const
        {
                return nodes_.begin();
        }
        auto end()
        {
                return nodes_.end();
        }
        auto end() const
        {
                return nodes_.end();
        }

        void clear_if( auto&& f )
        {
                nodes_.clear_if( f );
        }

        dag_node& emplace( const std::string& name, const task& t )
        {
                return nodes_.emplace_front( name, &t );
        }

        /// The DAG becames invalidated after any change to the input task set, and any use of it
        /// can result in undefined behavior.
        ///
        /// Function adds any dependencies of task into the `run_after` vector of each node, this
        /// way the execution functions does not have to handle dependencies explicitly for order of
        /// execution.
        ///
        /// \param ts A set of tasks for which the dag shall be generated
        /// \param filter Only nodes, and their dependencies, which match the filter are added to
        /// the dag.
        ///               Uses simple "find exact string match within a string" mechanism.
        ///
        void insert_set( const task_set& ts, const std::string& filter );

private:
        dag_node_list nodes_;
};

}  // namespace joque
