#include "joque/dag.hpp"

#include <set>

namespace joque
{
namespace
{

        template < typename T >
                requires( std::same_as< std::remove_cvref_t< T >, node > )
        void dfs( T& n, std::set< node* >& seen )
        {
                if ( seen.contains( &n ) ) {
                        return;
                }
                seen.insert( &n );
                for ( node* child : n.depends_on ) {
                        dfs( *child, seen );
                }
        }

        void link_dependencies( dag& g )
        {
                for ( node& n : g.nodes ) {
                        for ( const task* d : n.t->depends_on ) {
                                auto iter = std::ranges::find_if( g.nodes, [&]( const node& ch ) {
                                        return ch.t == d;
                                } );
                                if ( iter != g.nodes.end() ) {
                                        n.depends_on.push_back( &*iter );
                                        n.run_after.push_back( &*iter );
                                }
                        }
                }
        }

        void filter_nodes( dag& g, const std::string& filter )
        {
                std::set< node* > seen;
                for ( node& n : g.nodes ) {
                        if ( filter == "" || n.name.find( filter ) != std::string::npos ) {
                                dfs( n, seen );
                        }
                }
                std::erase_if( g.nodes, [&]( node& n ) {
                        return !seen.contains( &n );
                } );
        }

        void link_afters( dag& g )
        {
                for ( node& n : g.nodes ) {
                        for ( const task* a : n.t->run_after ) {
                                auto iter = std::ranges::find_if( g.nodes, [&]( const node& ch ) {
                                        return ch.t == a;
                                } );
                                if ( iter != g.nodes.end() ) {
                                        n.run_after.push_back( &*iter );
                                }
                        }
                }
        }
}  // namespace

dag generate_dag( const task_set& ts, const std::string& filter )
{
        dag g;
        for_each_task( ts, [&]( const std::string& name, const task& t ) {
                g.nodes.push_back( node{ .name = name, .t = &t } );
        } );

        link_dependencies( g );

        filter_nodes( g, filter );

        link_afters( g );

        return g;
}
}  // namespace joque
