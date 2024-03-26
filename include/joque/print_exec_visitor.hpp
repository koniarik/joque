#pragma once

#include "joque/dag.hpp"
#include "joque/exec_visitor.hpp"
#include "joque/records.hpp"

namespace joque
{

/// Default visitor for execution. Prints information on stdout or stderr.
class print_exec_visitor : public exec_visitor
{
public:
        print_exec_visitor( bool verbose = false, bool print_out = false );

        void on_node_enque( const dag_node& n ) override;
        void on_run_start( const dag_node& n ) override;
        void
        on_run_end( const exec_record& erec, const run_record* rec, const dag_node& n ) override;

        void on_exec_end( const exec_record& ) override;

private:
        bool verbose_;
        bool print_out_;
};

/// Global instnace of print visitor used as default argument for `exec`
extern print_exec_visitor PRINT_VISITOR;

}  // namespace joque
