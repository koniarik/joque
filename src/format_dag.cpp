
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
#include "joque/format_dag.hpp"

#include "joque/dag.hpp"

#include <functional>
#include <ostream>
#include <ranges>
#include <string>
#include <string_view>

namespace joque
{

namespace
{
        std::string join_names( const auto& edges )
        {
                std::string res;
                if ( edges.empty() )
                        return res;
                res = " " + edges.front()->target->name;
                for ( const dag_edge& e : edges | std::views::drop( 1 ) )
                        res += ", " + e->target->name;
                res += " ";

                return res;
        }
}  // namespace

void format_dag( const dag& d, const std::function< void( std::string_view ) >& f )
{

        for ( const dag_node& n : d ) {
                f( n->name );
                f( "   -> runs after {" + join_names( n.out_edges() ) + "}" );
        }
}

void print_dag( std::ostream& os, const dag& d )
{
        format_dag( d, [&]( std::string_view sv ) {
                os << sv << '\n';
        } );
}

}  // namespace joque
