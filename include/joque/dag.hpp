#pragma once

#include "joque/bits/dag.hpp"
#include "joque/bits/list.hpp"
#include "joque/task.hpp"

#include <iosfwd>
#include <memory>
#include <ranges>
#include <string>

namespace joque
{

struct edge_content;
using dag_edge = bits::gedge< edge_content >;

struct node_content;
using dag_node = bits::gnode< node_content, dag_edge >;

enum class ekind
{
        // A after B => A shall be executed after B finishes
        AFTER,
        // A invalidated_by B => if B was executed, A is considered out of date
        INVALIDATED_BY,
        // A requires B => A has to be skipped if B failed
        REQUIRES
};

std::ostream& operator<<( std::ostream& os, ekind k );

struct edge_content
{
        dag_node& source;
        dag_node& target;
        ekind     kind;
};

template < ekind Kind, typename Edges >
auto filter_edges( Edges& e )
{
        return e | std::views::filter( []( const dag_edge& e ) {
                       return e->kind == Kind;
               } );
}

template < typename Node, typename... Args >
void add_edge( Node& source, Node& target, Args&&... args )
{
        auto& e =
            source.out_edges().emplace_front( source, target, std::forward< Args >( args )... );
        target.in_edges().link_front( e );
}

enum class inval
{
        VALID,
        INVALID,
        UNKNOWN
};
std::ostream& operator<<( std::ostream& os, inval k );

struct node_content
{
        /// Full path-name of the task
        std::string name;
        /// Reference to the task
        const task& t;

        /// Node validation status
        inval invalidated = inval::UNKNOWN;

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
