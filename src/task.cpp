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
#include "joque/task.hpp"

#include <string>

namespace joque
{
void for_each_add_dep( task_set& ts, const task& dep )
{
        for_each_task( ts, [&]( const std::string&, task& t ) {
                if ( &t != &dep )
                        t.depends_on.emplace_back( dep );
        } );
}

void add_dep_to_each( task& t, const task_set& ts )
{
        for_each_task( ts, [&]( const std::string&, const task& other ) {
                if ( &t != &other )
                        t.depends_on.emplace_back( other );
        } );
}

void run_each_after( task_set& ts, const task& t )
{
        for_each_task( ts, [&]( const std::string&, task& other ) {
                if ( &other != &t )
                        other.run_after.emplace_back( t );
        } );
}

void run_after_all_of( task& t, const task_set& ts )
{
        for_each_task( ts, [&]( const std::string&, const task& other ) {
                if ( &t != &other )
                        t.run_after.emplace_back( other );
        } );
}

void invalidated_by_all_of( task& t, const task_set& ts )
{
        for_each_task( ts, [&]( const std::string&, const task& other ) {
                if ( &t != &other )
                        t.invalidated_by.emplace_back( other );
        } );
}

}  // namespace joque
