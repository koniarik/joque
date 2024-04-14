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
        virtual void after_node_enque( const dag_node& )
        {
        }

        virtual void on_detected_cycle( std::span< const dag_node* > )
        {
        }

        virtual void after_job_is_inval( const dag_node&, std::string_view )
        {
        }

        virtual void after_dep_inval( const dag_node&, const dag_node& )
        {
        }

        /// Executed once execution starts for the node.
        virtual void before_run( const dag_node& )
        {
        }

        virtual void on_run_log( const dag_node&, std::string_view )
        {
        }

        /// Executed after execution of node.
        /// \param rec If run produces a run record, it is passed, nullptr on errors
        virtual void after_run( const exec_record&, const run_record* /*rec*/, const dag_node& )
        {
        }

        virtual void after_execution( const exec_record& )
        {
        }

        virtual ~exec_visitor() = default;
};

}  // namespace joque
