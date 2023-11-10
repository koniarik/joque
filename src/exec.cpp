#include "joque/exec.hpp"

#include "joque/dag.hpp"
#include "run_coro.hpp"

#include <algorithm>
#include <chrono>
#include <future>
#include <set>

namespace joque
{
using namespace std::chrono_literals;

namespace
{

        void push( exec_record& erec, run_record rrec )
        {
                if ( rrec.skipped ) {
                        erec.skipped_count += 1;
                }
                if ( rrec.retcode != 0 ) {
                        erec.failed_count += 1;
                }
                erec.runs.push_back( std::move( rrec ) );
        }

        bool all_done( const auto& nodes )
        {
                return std::ranges::all_of( nodes, [&]( node* ch ) -> bool {
                        return ch->done;
                } );
        }

        bool any_failed( const auto& nodes )
        {
                return std::ranges::any_of( nodes, [&]( node* ch ) -> bool {
                        return ch->failed;
                } );
        }

        bool any_resource_used( const auto& resources, const auto& usage_set )
        {
                return std::ranges::any_of( resources, [&]( const resource* p ) {
                        return usage_set.contains( p );
                } );
        }

        node* find_candidate(
            node&                              n,
            std::set< node* >&                 seen,
            const std::set< const resource* >& used_res )
        {
                if ( n.started || n.done || seen.contains( &n ) ) {
                        return nullptr;
                }

                seen.insert( &n );

                for ( node* ch : n.run_after ) {
                        node* res = find_candidate( *ch, seen, used_res );
                        if ( res != nullptr ) {
                                return res;
                        }
                }

                if ( !all_done( n.run_after ) || any_failed( n.depends_on ) ||
                     any_resource_used( n.t->resources, used_res ) ) {
                        return nullptr;
                }
                return &n;
        }

        node* find_candidate(
            std::set< node* >&                 nodes,
            const std::set< const resource* >& used_resources )
        {
                node*             n = nullptr;
                std::set< node* > seen;
                for ( node* cand : nodes ) {
                        n = find_candidate( *cand, seen, used_resources );
                        if ( n != nullptr ) {
                                break;
                        }
                }
                return n;
        }

        bool is_invalidated( node& n )
        {
                const bool dep_invalidated =
                    std::ranges::any_of( n.depends_on, [&]( node* ch ) -> bool {
                            return ch->started;
                    } );

                return dep_invalidated || n.t->job->is_invalidated();
        }

        run_coro run( node& n, std::set< const resource* >& used_resources, unsigned thread_count )
        {
                run_record result{ .t = n.t, .name = n.name };

                if ( !is_invalidated( n ) ) {
                        n.done         = true;
                        result.skipped = true;
                        co_return result;
                }

                n.started = true;
                for ( const resource* r : n.t->resources ) {
                        used_resources.insert( r );
                }

                std::future< run_result > fut = std::async(
                    thread_count == 0 ? std::launch::deferred : std::launch::async,
                    []( node& n ) -> run_result {
                            run_result res;
                            try {
                                    res = n.t->job->run( n.t );
                            }
                            catch ( std::exception& e ) {
                                    res.retcode = 1;
                                    res.std_err += "job run failed due to exception: " +
                                                   std::string{ e.what() };
                            }
                            catch ( ... ) {
                                    res.retcode = 1;
                                    res.std_err += "job run failed due to an unknown exception";
                            }
                            return res;
                    },
                    std::ref( n ) );

                while ( fut.wait_for( 0ms ) == std::future_status::timeout ) {
                        co_await std::suspend_always{};
                }
                std::tie( result.retcode, result.std_out, result.std_err ) = fut.get();

                for ( const resource* r : n.t->resources ) {
                        used_resources.erase( r );
                }
                if ( result.retcode != 0 ) {
                        n.failed = true;
                }
                n.done     = true;
                result.end = std::chrono::system_clock::now();
                co_return result;
        }

        void cleanup_coros( std::vector< run_coro >& coros, exec_record& erec, exec_visitor& vis )
        {
                std::erase_if( coros, [&]( auto& coro ) {
                        if ( !coro.done() ) {
                                coro.tick();
                                return false;
                        }
                        run_record* rec = coro.result();
                        vis.on_run_end( rec, coro.get_node() );

                        if ( rec != nullptr ) {
                                push( erec, std::move( *rec ) );
                        }
                        return true;
                } );
        }
}  // namespace

exec_coro
exec( const task_set& ts, unsigned thread_count, const std::string& filter, exec_visitor& vis )
{

        dag g = generate_dag( ts, filter );
        return exec( std::move( g ), thread_count, vis );
}

exec_coro exec( dag g, unsigned thread_count, exec_visitor& vis )
{
        exec_record       erec;
        std::set< node* > to_process;
        for ( node& n : g.nodes ) {
                vis.on_node_enque( n );
                to_process.insert( &n );
        }

        std::set< const resource* > used_resources;
        std::vector< run_coro >     coros;

        while ( !to_process.empty() && erec.failed_count == 0 ) {
                cleanup_coros( coros, erec, vis );

                node* n = find_candidate( to_process, used_resources );
                if ( n != nullptr ) {
                        to_process.erase( n );

                        vis.on_run_start( *n );
                        coros.push_back( run( *n, used_resources, thread_count ) );
                }

                co_await std::suspend_always{};
        }
        while ( !coros.empty() ) {
                cleanup_coros( coros, erec, vis );
        }

        co_return erec;
}

}  // namespace joque
