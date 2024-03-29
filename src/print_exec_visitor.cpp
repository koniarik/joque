#include "joque/print_exec_visitor.hpp"

#include "joque/dag.hpp"
#include "joque/format.hpp"
#include "joque/records.hpp"
#include "joque/run_result.hpp"
#include "joque/traits.hpp"

#include <iostream>

namespace joque
{

print_exec_visitor::print_exec_visitor( bool verbose, bool print_out )
  : verbose_( verbose )
  , print_out_( print_out )
{
}

void print_exec_visitor::on_node_enque( const dag_node& n )
{
        if ( n->t.job == nullptr )
                std::cerr << "Task " << n->name << " has no job" << std::endl;
};

void print_exec_visitor::on_run_start( const dag_node& n )
{
        if ( verbose_ )
                std::cout << "Running task " << n->name << std::endl;
};

void print_exec_visitor::on_run_end(
    const exec_record& erec,
    const run_record*  rec,
    const dag_node&    n )
{
        if ( rec == nullptr ) {
                std::cerr << "Failed to get result from coro for task: " << n->name << std::endl;
                return;
        }
        if ( !rec->t.get().hidden || verbose_ || rec->retcode != 0 ) {
                format_record( std::cout, erec, *rec );
                std::cout.flush();
        }

        if ( print_out_ || verbose_ || rec->retcode != 0 )
                format_nested( std::cout, "        ", rec->output );
};

void print_exec_visitor::on_exec_end( const exec_record& rec )
{
        format_end( std::cout, rec );
}

print_exec_visitor PRINT_VISITOR;

}  // namespace joque
