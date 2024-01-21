#include "joque/dag.hpp"

#include <ranges>
#include <set>

namespace joque
{
namespace
{

        template < typename T >
                requires( std::same_as< std::remove_cvref_t< T >, dag_node > )
        void dfs( T& n, std::set< dag_node* >& seen )
        {
                if ( seen.contains( &n ) ) {
                        return;
                }
                seen.insert( &n );
                for ( dag_edge& e : n.runs_after | std::views::filter( []( const dag_edge& e ) {
                                            return e.is_dependency;
                                    } ) ) {
                        dfs( *e.target, seen );
                }
        }

        void link_dependencies( dag& g )
        {
                for ( dag_node& n : g.nodes ) {
                        for ( const task& d : n.t.get().depends_on ) {
                                auto iter =
                                    std::ranges::find_if( g.nodes, [&]( const dag_node& ch ) {
                                            // TODO: is pointer compare really wanted?
                                            return &ch.t.get() == &d;
                                    } );
                                if ( iter != g.nodes.end() ) {
                                        n.runs_after.emplace_front( true, &*iter );
                                }
                        }
                }
        }

        void filter_nodes( dag& g, const std::string& filter )
        {
                std::set< dag_node* > seen;
                for ( dag_node& n : g.nodes ) {
                        if ( filter == "" || n.name.find( filter ) != std::string::npos ) {
                                dfs( n, seen );
                        }
                }
                std::erase_if( g.nodes, [&]( dag_node& n ) {
                        return !seen.contains( &n );
                } );
        }

        void link_afters( dag& g )
        {
                for ( dag_node& n : g.nodes ) {
                        for ( const task& a : n.t.get().run_after ) {
                                auto iter =
                                    std::ranges::find_if( g.nodes, [&]( const dag_node& ch ) {
                                            return &ch.t.get() == &a;
                                    } );
                                if ( iter != g.nodes.end() ) {
                                        n.runs_after.emplace_front( false, &*iter );
                                }
                        }
                }
        }
}  // namespace

dag generate_dag( const task_set& ts, const std::string& filter )
{
        dag g;
        for_each_task( ts, [&]( const std::string& name, const task& t ) {
                g.nodes.push_back( dag_node{ .name = name, .t = t } );
        } );

        link_dependencies( g );

        filter_nodes( g, filter );

        link_afters( g );

        return g;
}
}  // namespace joque
