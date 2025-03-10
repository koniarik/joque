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

#include "dag.hpp"
#include "records.hpp"

namespace joque
{

/// Execution visitor interface, used by execution. Member functions are called
/// on events that occur during execution of tasks.
class exec_visitor
{
public:
        /// Run after node is enqued
        virtual void after_node_enque( const dag_node& )
        {
        }

        /// Ran in case cycle is detected
        virtual void on_detected_cycle( std::span< const dag_node* > )
        {
        }

        /// Run after check whenever job was invalidated
        virtual void after_job_is_inval( const dag_node&, std::string_view )
        {
        }

        /// Run after node became invalidated because of other node
        virtual void after_dep_inval( const dag_node&, const dag_node& )
        {
        }

        /// Executed once execution starts for the node.
        virtual void before_run( const exec_record&, const dag_node& )
        {
        }

        /// Run after job finished execution
        virtual void on_run_log( const dag_node&, std::string_view )
        {
        }

        /// Executed after execution of node.
        /// \param rec If run produces a run record, it is passed, nullptr on
        /// errors
        virtual void after_run( const exec_record&, const run_record* /*rec*/, const dag_node& )
        {
        }

        virtual void on_tick( const exec_record& )
        {
        }

        /// Run after execution finished
        virtual void after_execution( const exec_record& )
        {
        }

        virtual ~exec_visitor() = default;
};

}  // namespace joque
