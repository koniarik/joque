/// MIT License
///
/// Copyright (c) 2025 Jan Veverak Koniarik
///
/// Permission is hereby granted, free of charge, to any person obtaining a copy
/// of this software and associated documentation files (the "Software"), to deal
/// in the Software without restriction, including without limitation the rights
/// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
/// copies of the Software, and to permit persons to whom the Software is
/// furnished to do so, subject to the following conditions:
///
/// The above copyright notice and this permission notice shall be included in all
/// copies or substantial portions of the Software.
///
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
/// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
/// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
/// SOFTWARE.
#include "joque/print_exec_visitor.hpp"

#include "joque/dag.hpp"
#include "joque/format.hpp"
#include "joque/records.hpp"
#include "joque/traits.hpp"

#include <iostream>
#include <set>
#include <string_view>

namespace joque
{

struct print_exec_visitor::impl
{
        bool                        verbose;
        std::set< dag_node const* > queued;
};

print_exec_visitor::print_exec_visitor( bool verbose )
  : impl_( new print_exec_visitor::impl{
        .verbose = verbose,
    } )
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

void print_exec_visitor::after_job_is_inval( const dag_node& n, std::string_view log )
{
        if ( !impl_->verbose )
                return;

        if ( n->invalidated == inval::INVALID ) {
                std::cout << "Job " << n->name << " got invalidated: \n";
                std::cout << log << "\n";
        }
}

void print_exec_visitor::after_dep_inval( const dag_node& n, const dag_node& invalidator )
{
        if ( !impl_->verbose )
                return;

        std::cout << "Task " << n->name << " invalidated" << " by " << invalidator->name << '\n';
}

void print_exec_visitor::on_run_log( const dag_node& n, std::string_view log )

{
        if ( !impl_->verbose )
                return;
        std::cout << "Job " << n->name << " run log: \n";
        std::cout << log << "\n";
}

void print_exec_visitor::before_run( const exec_record& erec, const dag_node& n )
{
        if ( !n->t.hidden || impl_->verbose ) {
                impl_->queued.insert( &n );
                format_status( std::cout, erec, n->name );
                std::cout << "\033[?25l" << "\r" << "\033[?25h";
                std::cout.flush();
        }
};

void print_exec_visitor::after_run(
    const exec_record& erec,
    const run_record*  rec,
    const dag_node&    n )
{
        if ( rec == nullptr ) {
                std::cerr << "Failed to get result from coro for task: " << n->name << std::endl;
                return;
        }
        if ( !rec->t.get().hidden || impl_->verbose || rec->retcode != 0 ) {
                format_run_end( std::cout, erec, *rec );
                std::cout << std::endl;
        }

        if ( impl_->verbose || rec->retcode != 0 )
                format_nested( std::cout, "        ", rec->output );

        impl_->queued.erase( &n );
        if ( !impl_->queued.empty() ) {
                dag_node const* m = *impl_->queued.begin();
                format_status( std::cout, erec, ( *m )->name );
                std::cout << "\033[?25l" << "\r" << "\033[?25h";
                std::cout.flush();
        }
};

void print_exec_visitor::on_tick( const exec_record& )
{
}

void print_exec_visitor::after_execution( const exec_record& rec )
{

        std::cout << "\033[?25l" << "\r\033[?25h";
        format_exec_end( std::cout, rec );
        std::cout << std::endl;
        impl_->queued.clear();
}

print_exec_visitor::~print_exec_visitor() = default;

}  // namespace joque
