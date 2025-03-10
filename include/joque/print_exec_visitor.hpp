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
        print_exec_visitor( bool verbose = false );

        void after_node_enque( const dag_node& n ) override;

        void on_detected_cycle( std::span< const dag_node* > c ) override;

        void after_job_is_inval( const dag_node& n, std::string_view log ) override;

        void after_dep_inval( const dag_node& n, const dag_node& invalidator ) override;

        void before_run( const exec_record& erec, const dag_node& n ) override;

        void on_run_log( const dag_node& n, std::string_view log ) override;

        void
        after_run( const exec_record& erec, const run_record* rec, const dag_node& n ) override;

        void on_tick( const exec_record& erec ) override;

        void after_execution( const exec_record& ) override;

        ~print_exec_visitor();

private:
        struct impl;

        std::unique_ptr< impl > impl_;
};

/// Global instance of print visitor used as default argument for `exec`
inline print_exec_visitor PRINT_VISITOR;

}  // namespace joque
