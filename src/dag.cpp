#include "joque/dag.hpp"

#include "joque/task.hpp"

#include <ranges>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>

namespace joque
{
namespace
{

        template < typename T >
                requires( std::same_as< std::remove_cvref_t< T >, dag_node > )
        void dfs( T& n, std::unordered_set< dag_node* >& seen )
        {
                if ( seen.contains( &n ) )
                        return;
                seen.insert( &n );
                for ( const dag_edge& e :
                      n.out_edges() | std::views::filter( []( const dag_edge& e ) {
                              return e->is_dependency;
                      } ) ) {
                        dfs( e->target, seen );
                }
        }

        void link_dependencies( dag& g, std::unordered_map< const task*, dag_node* >& index )
        {
                for ( dag_node& n : g ) {
                        for ( const task& d : n->t.depends_on ) {
                                dag_node* target = index[&d];
                                dag_edge& e      = n.out_edges().emplace_front( true, n, *target );
                                target->in_edges().link_front( e );
                        }
                        for ( const task& d : n->t.run_after ) {
                                dag_node* target = index[&d];
                                dag_edge& e      = n.out_edges().emplace_front( false, n, *target );
                                target->in_edges().link_front( e );
                        }
                }
        }

        void filter_nodes( dag& g, const std::string& filter )
        {
                if ( filter == "" )
                        return;
                std::unordered_set< dag_node* > seen;
                for ( dag_node& n : g )
                        if ( n->name.find( filter ) != std::string::npos )
                                dfs( n, seen );

                g.clear_if( [&]( dag_node& node ) {
                        return !seen.contains( &node );
                } );
        }

}  // namespace

void insert_set( dag& dag, const task_set& ts, const std::string& filter )
{

        std::unordered_map< const task*, dag_node* > index;
        for_each_task( ts, [&]( const std::string& name, const task& t ) {
                index[&t] = &dag.emplace( name, t );
        } );

        link_dependencies( dag, index );
        filter_nodes( dag, filter );
}
}  // namespace joque
