#pragma once

#include "joque/dag.hpp"
#include "joque/exec_visitor.hpp"
#include "joque/records.hpp"

#include <iostream>

namespace joque
{

/// Default visitor for execution. Prints information on stdout or stderr.
class print_exec_visitor : public exec_visitor
{
public:
        print_exec_visitor( bool verbose = false, bool print_out = false );

        void after_node_enque( const dag_node& n ) override;

        void on_detected_cycle( std::span< const dag_node* > c ) override
        {
                std::cerr << "Cycle between nodes detected:\n";
                for ( auto* p : c )
                        std::cerr << "\t" << ( *p )->name << std::endl;
        }

        void after_job_is_inval( const dag_node& n, std::string_view log ) override
        {
                if ( !verbose_ )
                        return;

                if ( n->invalidated == inval::INVALID ) {
                        std::cout << "Job " << n->name << " got invalidated: \n";
                        std::cout << log << "\n";
                }
        }

        void after_dep_inval( const dag_node& n, const dag_node& invalidator ) override
        {
                if ( !verbose_ )
                        return;

                std::cout << "Task " << n->name << " invalidated"
                          << " by " << invalidator->name << '\n';
        }
        void before_run( const dag_node& n ) override;

        void on_run_log( const dag_node& n, std::string_view log ) override
        {
                if ( !verbose_ )
                        return;
                std::cout << "Job " << n->name << " run log: \n";
                std::cout << log << "\n";
        }

        void
        after_run( const exec_record& erec, const run_record* rec, const dag_node& n ) override;

        void after_execution( const exec_record& ) override;

private:
        bool verbose_;
        bool print_out_;
};

/// Global instnace of print visitor used as default argument for `exec`
extern print_exec_visitor PRINT_VISITOR;

}  // namespace joque
