
#include "joque/format_dag.hpp"

#include <ranges>
#include <string>

namespace joque
{

namespace
{
        std::string join_names( const auto& nodes )
        {
                std::string res;
                if ( nodes.empty() )
                        return res;
                res = " " + nodes.begin()->target->name;
                for ( const dag_edge& e : nodes | std::views::drop( 1 ) )
                        res += ", " + e.target->name;
                res += " ";

                return res;
        }
}  // namespace

void format_dag( const dag& d, const std::function< void( std::string_view ) >& f )
{

        for ( const dag_node& n : d ) {
                f( n.name );
                f( "   -> runs after {" + join_names( n.out_edges ) + "}" );
        }
}

void print_dag( std::ostream& os, const dag& d )
{
        format_dag( d, [&]( std::string_view sv ) {
                os << sv << '\n';
        } );
}

}  // namespace joque
