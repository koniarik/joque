#include "joque/print_exec_visitor.hpp"

#include "joque/format.hpp"
#include "joque/traits.hpp"

#include <iostream>

namespace joque
{

print_exec_visitor::print_exec_visitor( bool verbose )
  : verbose_( verbose )
{
}

void print_exec_visitor::on_node_enque( const dag_node& n )
{
        if ( n.t->job() == nullptr )
                std::cerr << "Task " << n.name << " has no job" << std::endl;
};

void print_exec_visitor::on_run_start( const dag_node& n )
{
        if ( verbose_ )
                std::cout << "Running task " << n.name << std::endl;
};

void print_exec_visitor::on_run_end( const run_record* rec, const dag_node& n )
{
        if ( rec == nullptr ) {
                std::cerr << "Failed to get result from coro for task: " << n.name << std::endl;
                return;
        }
        if ( !rec->t.get().is_hidden() || verbose_ ) {
                format_record( std::cout, *rec );
                std::cout.flush();
        }

        if ( verbose_ ) {
                for ( const output_chunk& ch : rec->output )
                        std::cout << ch.data;
                std::cout << std::endl;
        }
};

print_exec_visitor PRINT_VISITOR;

}  // namespace joque
