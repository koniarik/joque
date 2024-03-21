#include "joque/exec.hpp"

#include "joque/dag.hpp"
#include "joque/exec_coro.hpp"
#include "joque/exec_visitor.hpp"
#include "joque/records.hpp"
#include "joque/run_result.hpp"
#include "joque/task.hpp"
#include "joque/traits.hpp"
#include "run_coro.hpp"

#include <algorithm>
#include <bits/chrono.h>
#include <chrono>
#include <coroutine>
#include <exception>
#include <functional>
#include <future>
#include <set>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

namespace joque
{
using namespace std::chrono_literals;

namespace
{

        void push( exec_record& erec, run_record rrec )
        {
                if ( rrec.skipped )
                        erec.skipped_count += 1;
                if ( rrec.retcode != 0 )
                        erec.failed_count += 1;
                erec.runs.push_back( std::move( rrec ) );
        }

        bool all_ready( const auto& edges )
        {
                return std::ranges::all_of( edges, [&]( const dag_edge& e ) -> bool {
                        return e->target->done && !( e->is_dependency && e->target->failed );
                } );
        }

        bool any_resource_used( const auto& resources, const auto& usage_set )
        {
                return std::ranges::any_of( resources, [&]( const resource& p ) {
                        return usage_set.contains( &p );
                } );
        }

        dag_node* find_candidate(
            dag_node&                          n,
            std::set< dag_node* >&             seen,
            const std::set< const resource* >& used_res )
        {
                if ( n->started || n->done || seen.contains( &n ) )
                        return nullptr;

                seen.insert( &n );

                for ( const dag_edge& e : n.out_edges() ) {
                        dag_node* res = find_candidate( e->target, seen, used_res );
                        if ( res != nullptr )
                                return res;
                }

                if ( !all_ready( n.out_edges() ) || any_resource_used( n->t.resources, used_res ) )
                        return nullptr;
                return &n;
        }

        dag_node* find_candidate(
            std::set< dag_node* >&             nodes,
            const std::set< const resource* >& used_resources )
        {
                dag_node*             n = nullptr;
                std::set< dag_node* > seen;
                for ( dag_node* cand : nodes ) {
                        n = find_candidate( *cand, seen, used_resources );
                        if ( n != nullptr )
                                break;
                }
                return n;
        }

        bool is_invalidated( dag_node& n )
        {
                const bool dep_invalidated =
                    std::ranges::any_of( n.out_edges(), [&]( dag_edge& e ) -> bool {
                            return e->is_dependency && e->target->started;
                    } );

                return dep_invalidated || n->t.job->is_invalidated();
        }

        run_coro run( dag_node& n, std::set< const resource* >& used_resources, std::launch l )
        {
                run_record result{ .t = n->t, .name = n->name };

                if ( !is_invalidated( n ) ) {
                        n->done        = true;
                        result.skipped = true;
                        co_return result;
                }

                n->started = true;
                for ( const resource& r : n->t.resources )
                        used_resources.insert( &r );

                std::future< run_result > fut = std::async(
                    l,
                    []( dag_node& n ) -> run_result {
                            run_result res;
                            try {
                                    res = n->t.job->run( n->t );
                            }
                            catch ( std::exception& e ) {
                                    res.retcode = 1;
                                    record_output(
                                        res,
                                        output_chunk::ERROR,
                                        "job run failed due to exception: " +
                                            std::string{ e.what() } );
                            }
                            catch ( ... ) {
                                    res.retcode = 1;
                                    record_output(
                                        res,
                                        output_chunk::ERROR,
                                        "job run failed due to an unknown exception" );
                            }
                            return res;
                    },
                    std::ref( n ) );

                while ( fut.wait_for( 0ms ) == std::future_status::timeout )
                        co_await std::suspend_always{};
                std::tie( result.retcode, result.output ) = fut.get();

                for ( const resource& r : n->t.resources )
                        used_resources.erase( &r );
                if ( result.retcode != 0 )
                        n->failed = true;
                n->done    = true;
                result.end = std::chrono::system_clock::now();
                co_return result;
        }

        void cleanup_coros( std::vector< run_coro >& coros, exec_record& erec, exec_visitor& vis )
        {
                std::erase_if( coros, [&]( auto& coro ) {
                        coro.tick();
                        if ( !coro.done() )
                                return false;
                        run_record* rec = coro.result();
                        vis.on_run_end( erec, rec, coro.get_node() );

                        if ( rec != nullptr )
                                push( erec, std::move( *rec ) );
                        return true;
                } );
        }
}  // namespace

exec_coro
exec( const task_set& ts, unsigned thread_count, const std::string& filter, exec_visitor& vis )
{
        dag g;
        insert_set( g, ts, filter );
        return exec( std::move( g ), thread_count, vis );
}

exec_coro exec( dag g, unsigned thread_count, exec_visitor& vis )
{
        std::set< dag_node* > to_process;
        for ( dag_node& n : g ) {
                vis.on_node_enque( n );
                to_process.insert( &n );
        }
        exec_record erec{ .total_count = to_process.size() };

        std::set< const resource* > used_resources;
        std::vector< run_coro >     coros;

        while ( !to_process.empty() ) {
                cleanup_coros( coros, erec, vis );

                dag_node* n = find_candidate( to_process, used_resources );
                if ( n != nullptr ) {
                        to_process.erase( n );

                        vis.on_run_start( *n );
                        coros.push_back(
                            run( *n,
                                 used_resources,
                                 thread_count == 0 ? std::launch::deferred : std::launch::async ) );
                }

                co_await std::suspend_always{};
        }
        while ( !coros.empty() )
                cleanup_coros( coros, erec, vis );

        vis.on_exec_end( erec );

        co_return erec;
}

}  // namespace joque
