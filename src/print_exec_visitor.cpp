#include "joque/print_exec_visitor.hpp"

#include "joque/dag.hpp"
#include "joque/format.hpp"
#include "joque/records.hpp"
#include "joque/traits.hpp"

#include <iostream>
#include <string_view>

namespace joque
{

print_exec_visitor::print_exec_visitor( bool verbose )
  : verbose_( verbose )
{
}

void print_exec_visitor::after_node_enque( const dag_node& n )
{
        if ( n->t.job == nullptr )
                std::cerr << "Task " << n->name << " has no job" << std::endl;
};

void print_exec_visitor::on_detected_cycle( std::span< const dag_node* > c )
{
        std::cerr << "Cycle between nodes detected:\n";
        for ( auto* p : c )
                std::cerr << "\t" << ( *p )->name << std::endl;
}

void print_exec_visitor::after_job_is_inval(
    const dag_node&  n,
    std::string_view log )
{
        if ( !verbose_ )
                return;

        if ( n->invalidated == inval::INVALID ) {
                std::cout << "Job " << n->name << " got invalidated: \n";
                std::cout << log << "\n";
        }
}

void print_exec_visitor::after_dep_inval(
    const dag_node& n,
    const dag_node& invalidator )
{
        if ( !verbose_ )
                return;

        std::cout << "Task " << n->name << " invalidated"
                  << " by " << invalidator->name << '\n';
}

void print_exec_visitor::on_run_log( const dag_node& n, std::string_view log )

{
        if ( !verbose_ )
                return;
        std::cout << "Job " << n->name << " run log: \n";
        std::cout << log << "\n";
}

void print_exec_visitor::before_run(
    const exec_record& erec,
    const dag_node&    n )
{
        format_run_start( std::cout, erec, n->name );
        std::cout << "\033[?25l" << "\r\033[?25h";
        std::cout.flush();
};

void print_exec_visitor::after_run(
    const exec_record& erec,
    const run_record*  rec,
    const dag_node&    n )
{
        if ( rec == nullptr ) {
                std::cerr << "Failed to get result from coro for task: "
                          << n->name << std::endl;
                return;
        }
        if ( !rec->t.get().hidden || verbose_ || rec->retcode != 0 ) {
                format_run_end( std::cout, erec, *rec );
                std::cout << std::endl;
        }

        if ( verbose_ || rec->retcode != 0 )
                format_nested( std::cout, "        ", rec->output );
};

void print_exec_visitor::after_execution( const exec_record& rec )
{

        std::cout << "\033[?25l" << "\r\033[?25h";
        format_exec_end( std::cout, rec );
        std::cout << std::endl;
}

}  // namespace joque
