#pragma once

#include "joque/bits/dag.hpp"
#include "joque/bits/list.hpp"
#include "joque/task.hpp"

#include <memory>
#include <string>

namespace joque
{

struct edge_content;
using dag_edge = bits::gedge< edge_content >;

struct node_content;
using dag_node = bits::gnode< node_content, dag_edge >;

struct edge_content
{
        bool      is_dependency = false;
        dag_node& source;
        dag_node& target;
};

struct node_content
{
        /// Full path-name of the task
        std::string name;
        /// Reference to the task
        const task& t;

        /// Sets to `true` once the task is scheduled for execution
        bool started = false;
        /// Sets to `done` if the task finished it's execution (correctly or incorrectly)
        bool done = false;
        /// Sets to `fail` if the task failed during execution
        bool failed = false;
};


/// DAG used to store data in single execution of tasks
using dag = bits::graph< dag_node >;

void insert_set( dag& dag, const task_set& ts, const std::string& filter );

}  // namespace joque
