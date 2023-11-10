#pragma once

#include "joque/exec_visitor.hpp"

namespace joque
{

/// Default visitor for execution. Prints information on stdout or stderr.
class print_exec_visitor : public exec_visitor
{
public:
        void on_node_enque( const node& n ) override;
        void on_run_start( const node& n ) override;
        void on_run_end( const run_record* rec, const node& n ) override;
};

/// Global instnace of print visitor used as default argument for `exec`
extern print_exec_visitor PRINT_VISITOR;

}  // namespace joque
