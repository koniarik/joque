#include "joque/dag.hpp"

#include <set>

namespace joque
{

template < typename T >
        requires( std::same_as< std::remove_cvref_t< T >, node > )
void dfs( T& n, std::set< node* >& seen )
{
        seen.insert( &n );
        for ( node* child : n.depends_on ) {
                if ( seen.contains( child ) ) {
                        continue;
                }
                dfs( *child, seen );
        }
}

dag generate_dag( const task_set& ts, const std::string& filter )
{
        dag g;
        for_each_task( ts, [&]( const std::string& name, const task& t ) {
                g.nodes.push_back( node{ .name = name, .t = &t } );
        } );
        for ( node& n : g.nodes ) {
                for ( const task* d : n.t->deps ) {
                        auto iter = std::ranges::find_if( g.nodes, [&]( const node& ch ) {
                                return ch.t == d;
                        } );
                        if ( iter != g.nodes.end() ) {
                                n.depends_on.push_back( &*iter );
                                n.after.push_back( &*iter );
                        }
                }
        }
        std::set< node* > seen;
        for ( node& n : g.nodes ) {
                if ( filter == "" || n.name.find( filter ) != std::string::npos ) {
                        dfs( n, seen );
                }
        }
        std::erase_if( g.nodes, [&]( node& n ) {
                return !seen.contains( &n );
        } );
        for ( node& n : g.nodes ) {
                for ( const task* a : n.t->after ) {
                        auto iter = std::ranges::find_if( g.nodes, [&]( const node& ch ) {
                                return ch.t == a;
                        } );
                        if ( iter != g.nodes.end() ) {
                                n.after.push_back( &*iter );
                        }
                }
        }
        return g;
}
}  // namespace joque
