/// MIT License
///
/// Copyright (c) 2025 Jan Veverak Koniarik
///
/// Permission is hereby granted, free of charge, to any person obtaining a copy
/// of this software and associated documentation files (the "Software"), to deal
/// in the Software without restriction, including without limitation the rights
/// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
/// copies of the Software, and to permit persons to whom the Software is
/// furnished to do so, subject to the following conditions:
///
/// The above copyright notice and this permission notice shall be included in all
/// copies or substantial portions of the Software.
///
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
/// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
/// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
/// SOFTWARE.
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
#include <cassert>
#include <chrono>
#include <coroutine>
#include <exception>
#include <functional>
#include <future>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace joque
{
using namespace std::chrono_literals;

namespace
{

        void push( exec_record& erec, run_record rrec )
        {
                erec.stats[rrec.status] += 1;
                erec.runs.push_back( std::move( rrec ) );
        }

        bool all_done( auto& edges )
        {
                return std::ranges::all_of( edges, [&]( const dag_edge& e ) -> bool {
                        return e->target->done;
                } );
        }

        bool any_dep_failed( const auto& edges )
        {
                return std::ranges::any_of(
                    filter_edges< ekind::REQUIRES >( edges ), [&]( const dag_edge& e ) {
                            return e->target->failed;
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

                auto es = filter_edges< ekind::AFTER >( n.out_edges() );
                for ( const dag_edge& e : es ) {
                        dag_node* res = find_candidate( e->target, seen, used_res );
                        if ( res != nullptr )
                                return res;
                }

                if ( !all_done( es ) || any_resource_used( n->t.resources, used_res ) )
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

        run_coro
        run( dag_node&                    n,
             exec_record const&           erec,
             std::set< const resource* >& used_resources,
             std::launch                  l,
             exec_visitor&                vis )
        {
                run_record result{ .t = n->t, .name = n->name };

                assert( n->invalidated != inval::UNKNOWN );

                vis.before_run( erec, n );

                if ( any_dep_failed( n.out_edges() ) ) {
                        n->done       = true;
                        n->failed     = true;
                        result.status = run_status::DEPF;
                        // TODO: fixx
                        // for ( auto& e : filter_edges< ekind::REQUIRES >(
                        // n.out_edges() ) )
                        //       result.log.emplace_back( "Failed dep: " +
                        //       e->target->name );
                        co_return result;
                }
                if ( n->invalidated == inval::VALID ) {
                        n->done       = true;
                        result.status = run_status::SKIP;
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
                                    insert_err(
                                        res,
                                        "job run failed due to exception: " +
                                            std::string{ e.what() } );
                            }
                            catch ( ... ) {
                                    res.retcode = 1;
                                    insert_err( res, "job run failed due to an unknown exception" );
                            }
                            return res;
                    },
                    std::ref( n ) );

                while ( fut.wait_for( 0ms ) == std::future_status::timeout )
                        co_await std::suspend_always{};
                run_result rr  = fut.get();
                result.retcode = rr.retcode;
                result.output  = std::move( rr.output );
                vis.on_run_log( n, rr.log );

                for ( const resource& r : n->t.resources )
                        used_resources.erase( &r );
                if ( result.retcode != 0 ) {
                        n->failed     = true;
                        result.status = run_status::FAIL;
                }
                n->done    = true;
                result.end = std::chrono::system_clock::now();
                co_return result;
        }

        void cleanup_coros( std::vector< run_coro >& coros, exec_record& erec, exec_visitor& vis )
        {
                std::erase_if( coros, [&]( run_coro& coro ) {
                        coro.tick();
                        if ( !coro.done() )
                                return false;
                        run_record* rec = coro.result();

                        vis.after_run( erec, rec, coro.get_node() );

                        if ( rec != nullptr )
                                push( erec, std::move( *rec ) );
                        return true;
                } );
        }

        void propagate_invalidation( std::set< dag_node* >& to_process, exec_visitor& vis )
        {
                std::set< dag_node* > to_propagate;
                for ( dag_node* p : to_process ) {
                        dag_node&    n    = *p;
                        inval_result ires = n->t.job->is_invalidated();
                        if ( ires.invalidated ) {
                                n->invalidated = inval::INVALID;
                                to_propagate.insert( &n );
                        } else {
                                n->invalidated = inval::VALID;
                        }
                        vis.after_job_is_inval( n, ires.log );
                }
                std::set< dag_node* > seen = to_propagate;
                while ( !to_propagate.empty() ) {
                        dag_node* p = to_propagate.extract( to_propagate.begin() ).value();
                        seen.insert( p );
                        assert( ( *p )->invalidated == inval::INVALID );
                        for ( dag_edge& e :
                              filter_edges< ekind::INVALIDATED_BY >( p->in_edges() ) ) {

                                if ( seen.contains( &e->source ) )
                                        continue;

                                e->source->invalidated = inval::INVALID;
                                to_propagate.insert( &e->source );
                                vis.after_dep_inval( e->source, e->target );
                        }
                }
        }

        std::vector< const dag_node* >
        get_cycle( const std::vector< dag_node* >& stack, dag_node& n )
        {
                auto                           iter = std::ranges::find( stack, &n );
                std::vector< const dag_node* > cycle;
                cycle.reserve( stack.size() );
                while ( iter != stack.end() )
                        cycle.push_back( &( **iter++ ) );
                return cycle;
        }

        dag_node* check_for_cycle(
            dag_node&                 n,
            std::set< dag_node* >&    seen,
            std::vector< dag_node* >& stack,
            exec_visitor&             vis )
        {
                auto [it1, not_seen] = seen.insert( &n );
                bool on_stack        = std::ranges::find( stack, &n ) != stack.end();
                if ( !not_seen ) {
                        bool has_c = !not_seen && on_stack;
                        return has_c ? &n : nullptr;
                }
                stack.push_back( &n );
                auto es = filter_edges< ekind::AFTER >( n.out_edges() );
                for ( auto& e : es )
                        if ( dag_node* p = check_for_cycle( e->target, seen, stack, vis ) ) {
                                if ( p != &n )
                                        return p;
                                std::vector< const dag_node* > cycle = get_cycle( stack, *p );
                                vis.on_detected_cycle( cycle );
                                return p;
                        }
                stack.pop_back();
                return nullptr;
        }

        bool has_cycle( const std::set< dag_node* >& to_process, exec_visitor& vis )
        {
                std::set< dag_node* >    seen;
                std::vector< dag_node* > stack;

                return std::ranges::any_of( to_process, [&]( dag_node* n ) {
                        assert( stack.empty() );
                        return check_for_cycle( *n, seen, stack, vis ) != nullptr;
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
        exec_record           erec;
        std::set< dag_node* > to_process;
        for ( dag_node& n : g ) {
                to_process.insert( &n );
                vis.after_node_enque( n );
        }
        erec.total_count = to_process.size();

        if ( has_cycle( to_process, vis ) )
                throw std::runtime_error( "Cycle detected" );
        propagate_invalidation( to_process, vis );

        std::set< const resource* > used_resources;
        std::vector< run_coro >     coros;

        while ( !to_process.empty() ) {
                vis.on_tick( erec );
                cleanup_coros( coros, erec, vis );
                if ( coros.size() >= thread_count && thread_count != 0 ) {
                        co_await std::suspend_always{};
                        continue;
                }

                dag_node* n = find_candidate( to_process, used_resources );
                if ( n != nullptr ) {
                        to_process.erase( n );

                        coros.push_back(
                            run( *n,
                                 erec,
                                 used_resources,
                                 thread_count == 0 ? std::launch::deferred : std::launch::async,
                                 vis ) );
                }
                assert( !coros.empty() );
                co_await std::suspend_always{};
        }
        while ( !coros.empty() ) {
                cleanup_coros( coros, erec, vis );
                co_await std::suspend_always{};
        }

        vis.after_execution( erec );

        co_return erec;
}

}  // namespace joque
