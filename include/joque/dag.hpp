/// MIT License
///
/// Copyright (c) 2025 Jan Veverak Koniarik
///
/// Permission is hereby granted, free of charge, to any person obtaining a copy
/// of this software and associated documentation files (the "Software"), to deal
/// in the Software without restriction, including without limitation the rights
/// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
/// copies of the Software, and to permit persons to whom the Software is
/// furnished to do so, subject to the following conditions:
///
/// The above copyright notice and this permission notice shall be included in all
/// copies or substantial portions of the Software.
///
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
/// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
/// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
/// SOFTWARE.
#pragma once

#include "bits/dag.hpp"
#include "bits/list.hpp"
#include "task.hpp"

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

enum class ekind : uint8_t
{
        // A after B => A shall be executed after B finishes
        AFTER,
        // A invalidated_by B => if B was executed, A is considered out of date
        INVALIDATED_BY,
        // A requires B => A has to be skipped if B failed
        REQUIRES
};

std::string_view to_sv( const ekind& e );

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

enum class inval : uint8_t
{
        VALID,
        INVALID,
        UNKNOWN
};
std::string_view to_sv( const inval& k );

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
        /// Sets to `done` if the task finished it's execution (correctly or
        /// incorrectly)
        bool done = false;
        /// Sets to `fail` if the task failed during execution
        bool failed = false;
};

/// DAG used to store data in single execution of tasks
using dag = bits::graph< dag_node >;

void insert_set( dag& dag, const task_set& ts, const std::string& filter );

}  // namespace joque
