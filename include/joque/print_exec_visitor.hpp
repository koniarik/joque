#pragma once

#include "dag.hpp"
#include "exec_visitor.hpp"
#include "records.hpp"

#include <iostream>
#include <span>
#include <string_view>

namespace joque
{

/// Default visitor for execution. Prints information on stdout or stderr.
class print_exec_visitor : public exec_visitor
{
public:
        print_exec_visitor( bool verbose = false, bool print_out = false );

        void after_node_enque( const dag_node& n ) override;

        void on_detected_cycle( std::span< const dag_node* > c ) override;

        void
        after_job_is_inval( const dag_node& n, std::string_view log ) override;

        void after_dep_inval( const dag_node& n, const dag_node& invalidator )
            override;

        void before_run( const dag_node& n ) override;

        void on_run_log( const dag_node& n, std::string_view log ) override;

        void after_run(
            const exec_record& erec,
            const run_record*  rec,
            const dag_node&    n ) override;

        void after_execution( const exec_record& ) override;

private:
        bool verbose_;
        bool print_out_;
};

/// Global instance of print visitor used as default argument for `exec`
inline print_exec_visitor PRINT_VISITOR;

}  // namespace joque
