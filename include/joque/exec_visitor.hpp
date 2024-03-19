#pragma once

#include "joque/dag.hpp"
#include "joque/records.hpp"

namespace joque
{

/// Execution visitor interface, used by execution. Member functions are called on events that occur
/// during execution of tasks.
class exec_visitor
{
public:
        /// Executed before node is enqued into internal queue of execution.
        virtual void on_node_enque( const dag_node& )
        {
        }

        /// Executed once execution starts for the node.
        virtual void on_run_start( const dag_node& )
        {
        }

        /// Executed after execution of node.
        /// \param rec If run produces a run record, it is passed, nullptr on errors
        virtual void on_run_end( const exec_record&, const run_record* /*rec*/, const dag_node& )
        {
        }

        virtual void on_exec_end( const exec_record& )
        {
        }

        virtual ~exec_visitor() = default;
};

}  // namespace joque
