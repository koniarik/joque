
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

void format_dag(
    const dag&                                       d,
    const std::function< void( std::string_view ) >& f )
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
