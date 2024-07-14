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
        virtual void after_run(
            const exec_record&,
            const run_record* /*rec*/,
            const dag_node& )
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
