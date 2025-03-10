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
#include "joque/dag.hpp"

#include "joque/task.hpp"

#include <ranges>
#include <string>
#include <string_view>
#include <unordered_map>

namespace joque
{
namespace
{

        void dfs( dag& dag, const task& t, auto& tmp, auto& index )
        {
                if ( index.contains( &t ) )
                        return;
                index[&t] = &dag.emplace( tmp[&t], t );

                for ( const task& d : t.depends_on ) {
                        dfs( dag, d, tmp, index );

                        add_edge( *index[&t], *index[&d], ekind::AFTER );
                        add_edge( *index[&t], *index[&d], ekind::REQUIRES );
                        add_edge( *index[&t], *index[&d], ekind::INVALIDATED_BY );
                }
        }
}  // namespace

std::string_view to_sv( const ekind& e )
{
        switch ( e ) {
        case ekind::AFTER:
                return "after";
        case ekind::INVALIDATED_BY:
                return "invalidated by";
        case ekind::REQUIRES:
                return "requires";
        }
        return "";
}

std::string_view to_sv( const inval& k )
{
        switch ( k ) {
        case inval::VALID:
                return "valid";
        case inval::INVALID:
                return "invalid";
        case inval::UNKNOWN:
                return "unknown";
        }
        return "";
}

void insert_set( dag& dag, const task_set& ts, const std::string& filter )
{

        std::unordered_map< const task*, std::string > tmp;
        for_each_task( ts, [&]( const std::string& name, const task& t ) {
                tmp.emplace( &t, name );
        } );

        std::unordered_map< const task*, dag_node* > index;
        for ( auto& [t, n] : tmp ) {
                if ( n.find( filter ) == std::string::npos )
                        continue;
                dfs( dag, *t, tmp, index );
        }

        for ( auto&& [t, n] : index ) {
                for ( const task& d : t->run_after ) {
                        if ( !index.contains( &d ) )
                                continue;
                        add_edge( *n, *index[&d], ekind::AFTER );
                }
                for ( const task& d : t->invalidated_by ) {
                        if ( !index.contains( &d ) )
                                continue;
                        add_edge( *n, *index[&d], ekind::INVALIDATED_BY );
                }
        }
}
}  // namespace joque
