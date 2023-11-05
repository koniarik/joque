#include "joque/exec.hpp"

#include "joque/dag.hpp"
#include "run_coro.hpp"

#include <algorithm>
#include <chrono>
#include <future>
#include <iostream>
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

                const bool children_are_done =
                    std::ranges::all_of( n.run_after, [&]( node* ch ) -> bool {
                            return ch->done;
                    } );
                if ( !children_are_done ) {
                        return nullptr;
                }
                const bool has_failed_child =
                    std::ranges::any_of( n.depends_on, [&]( node* ch ) -> bool {
                            return ch->failed;
                    } );
                if ( has_failed_child ) {
                        return nullptr;
                }

                const bool res_used =
                    std::ranges::any_of( n.t->resources, [&]( const resource* p ) {
                            return used_res.contains( p );
                    } );
                if ( res_used ) {
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
                const bool job_invalidated = n.t->job->is_invalidated();

                return dep_invalidated || job_invalidated;
        }

        auto msg( const std::string& state, const std::string& name )
        {
                return std::format( " {:>5}  {:<60} ", state, name );
        }

        void print_rec( const run_record& rec )
        {
                std::cout << msg(
                    rec.skipped      ? "SKIPPED" :
                    rec.retcode == 0 ? "OK" :
                                       "FAIL",
                    rec.name );
                if ( !rec.skipped ) {
                        std::cout << std::format(
                            "({})",
                            std::chrono::duration_cast< std::chrono::milliseconds >(
                                rec.end - rec.start ) );
                }
                std::cout << "\n";
                if ( rec.retcode != 0 ) {
                        std::istringstream std_out{ rec.std_out };
                        for ( std::string line; std::getline( std_out, line ); ) {
                                std::cout << "      " << line << "\n";
                        }
                        std::istringstream std_err{ rec.std_err };
                        for ( std::string line; std::getline( std_err, line ); ) {
                                std::cout << "      " << line << "\n";
                        }
                }
                std::cout.flush();
        }

        run_coro
        run( node&                        n,
             std::condition_variable&     cv,
             std::set< const resource* >& used_resources,
             unsigned                     thread_count )
        {
                run_record result;
                result.t    = n.t;
                result.name = n.name;

                n.started = true;

                if ( !is_invalidated( n ) ) {
                        n.done         = true;
                        result.skipped = true;
                        co_return result;
                }

                for ( const resource* r : n.t->resources ) {
                        used_resources.insert( r );
                }
                std::future< run_result > fut = std::async(
                    thread_count == 0 ? std::launch::deferred : std::launch::async,
                    [&]( node& n ) -> run_result {
                            run_result res;
                            try {
                                    res = n.t->job->run( n.t );
                            }
                            catch ( std::exception& e ) {
                                    res.retcode = 1;
                                    res.serr += "job run failed due to exception: " +
                                                std::string{ e.what() };
                            }
                            catch ( ... ) {
                                    res.retcode = 1;
                                    res.serr += "job run failed due to an unknown exception";
                            }
                            cv.notify_one();
                            return res;
                    },
                    std::ref( n ) );

                while ( fut.wait_for( 0ms ) == std::future_status::timeout ) {
                        co_await std::suspend_always{};
                }
                std::tie( result.retcode, result.std_out, result.std_err ) = fut.get();

                n.done = true;
                for ( const resource* r : n.t->resources ) {
                        used_resources.erase( r );
                }
                if ( result.retcode != 0 ) {
                        n.failed = true;
                }
                result.end = std::chrono::system_clock::now();
                co_return result;
        }

        void cleanup_coros( std::vector< run_coro >& coros, exec_record& erec )
        {
                std::erase_if( coros, [&]( auto& coro ) {
                        if ( !coro.done() ) {
                                coro.tick();
                                return false;
                        }
                        run_record* rec = coro.result();
                        if ( rec == nullptr ) {
                                std::cerr << "Failed to get result from coro for task: "
                                          << coro.get_node().name << std::endl;
                                return true;
                        }
                        if ( !rec->t->hidden ) {
                                print_rec( *rec );
                        }
                        push( erec, std::move( *rec ) );
                        return true;
                } );
        }
}  // namespace

exec_coro exec( const task_set& ts, unsigned thread_count, const std::string& filter )
{

        dag g = generate_dag( ts, filter );
        return exec( std::move( g ), thread_count, filter );
}

exec_coro exec( dag g, unsigned thread_count, const std::string& filter )
{
        std::condition_variable cv;
        std::mutex              m;

        exec_record       erec;
        std::set< node* > to_process;
        for ( node& n : g.nodes ) {
                if ( n.t->job == nullptr ) {
                        std::cerr << "Task " << n.name << " has no job" << std::endl;
                }
                to_process.insert( &n );
        }

        std::set< const resource* > used_resources;
        std::vector< run_coro >     coros;

        while ( !to_process.empty() && erec.failed_count == 0 ) {
                cleanup_coros( coros, erec );

                std::unique_lock lk( m );

                node* n = find_candidate( to_process, used_resources );
                if ( n != nullptr ) {
                        to_process.erase( n );

                        coros.push_back( run( *n, cv, used_resources, thread_count ) );
                }

                co_await std::suspend_always{};
        }
        while ( !coros.empty() ) {
                cleanup_coros( coros, erec );
        }

        co_return erec;
}

}  // namespace joque
