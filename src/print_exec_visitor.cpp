#include "joque/print_exec_visitor.hpp"

#include "joque/format.hpp"

#include <iostream>

namespace joque
{

void print_exec_visitor::on_node_enque( const node& n )
{
        if ( n.t->job == nullptr ) {
                std::cerr << "Task " << n.name << " has no job" << std::endl;
        }
};

void print_exec_visitor::on_run_start( const node& ){};

void print_exec_visitor::on_run_end( const run_record* rec, const node& n )
{
        if ( rec == nullptr ) {
                std::cerr << "Failed to get result from coro for task: " << n.name << std::endl;
                return;
        }
        if ( !rec->t->hidden ) {
                format_record( std::cout, *rec );
                std::cout.flush();
        }
};

print_exec_visitor PRINT_VISITOR;

}  // namespace joque
