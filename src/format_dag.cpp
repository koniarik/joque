
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
                if ( nodes.empty() ) {
                        return res;
                }
                res = " " + nodes.front().get().name;
                for ( const node& n : nodes | std::views::drop( 1 ) ) {
                        res += ", " + n.name;
                }
                res += " ";

                return res;
        }
}  // namespace

void format_dag( const dag& d, const std::function< void( std::string_view ) >& f )
{

        for ( const node& n : d.nodes ) {
                f( n.name );
                f( "   -> depends on {" + join_names( n.depends_on ) + "}" );
                f( "   -> runs after {" + join_names( n.run_after ) + "}" );
        }
}

void print_dag( std::ostream& os, const dag& d )
{
        format_dag( d, [&]( std::string_view sv ) {
                os << sv << '\n';
        } );
}

}  // namespace joque
