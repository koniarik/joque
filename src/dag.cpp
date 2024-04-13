#include "joque/dag.hpp"

#include "joque/task.hpp"

#include <ostream>
#include <ranges>
#include <string>
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
