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
#include "joque/process.hpp"
#include "joque/run_result.hpp"
#include "joque/task.hpp"
#include "joque/traits.hpp"

#include <algorithm>
#include <gtest/gtest.h>
#include <mutex>
#include <numeric>
#include <ranges>
#include <set>
#include <string>
#include <vector>

namespace joque
{

TEST( joque, compile_test )
{
        resource my_dev{ .name = "my_device" };
        task_set ts{};

        task& t1 =
            ( ts.tasks["my_test"] = task{
                  .job       = process::derive( "ls", "./", out( "/tmp/out.txt" ) ),
                  .resources = { my_dev },
              } );

        ts.tasks["my_test2"] = task{
            .job = []( const task& ) -> run_result {
                    return { 0 };
            },
            .depends_on = { t1 },
            .resources  = { my_dev },
        };
}

TEST( joque, basic )
{

        std::vector< int > sequence( 10 );
        std::iota( sequence.begin(), sequence.end(), 0 );
        std::vector< int > result;
        std::mutex         w_m;

        task_set ts{};
        for ( const int i : sequence ) {
                ts.tasks["my_test_" + std::to_string( i )] = task{
                    .job = [&, i = i]( const task& ) -> run_result {
                            const std::lock_guard _{ w_m };
                            result.push_back( i );
                            return { 0 };
                    },
                };
        }

        for ( const unsigned i : { 0u, 4u } ) {
                result.clear();
                exec( ts, i ).run();
                std::sort( result.begin(), result.end() );

                EXPECT_EQ( sequence, result ) << "thread count: " << i;
        }
}

TEST( joque, dep )
{
        task_set                ts{};
        std::mutex              m;
        std::set< const task* > finished;

        auto f = [&]( const task& t ) -> run_result {
                const std::lock_guard _{ m };
                for ( const task& dep : t.depends_on )
                        EXPECT_TRUE( finished.contains( &dep ) );
                finished.insert( &t );
                return { 0 };
        };

        const task* last =
            &( ts.tasks["my_test"] = task{
                   .job = f,
               } );
        for ( const std::size_t i : std::views::iota( 0u, 10u ) ) {
                ts.tasks["my_test_" + std::to_string( i ) + "_a"] = task{
                    .job        = f,
                    .depends_on = { *last },
                };
                last =
                    &( ts.tasks["my_test_" + std::to_string( i ) + "_b"] = task{
                           .job        = f,
                           .depends_on = { *last },
                       } );
        }

        for ( const unsigned i : { 0u, 4u } ) {
                finished.clear();
                exec( ts, i ).run();
                EXPECT_EQ( finished.size(), ts.tasks.size() ) << "thread count: " << i;
        }
}

TEST( joque, depf )
{
        task_set    ts{};
        std::size_t counter = 0;
        std::mutex  m;

        auto f = [&]( const task& ) -> run_result {
                const std::lock_guard _{ m };
                counter += 1;
                return { 1 };
        };

        ts.tasks["a"] = task{ .job = f };
        ts.tasks["b"] = task{ .job = f, .depends_on = { ts.tasks["a"] } };
        ts.tasks["c"] = task{ .job = f, .depends_on = { ts.tasks["b"] } };

        for ( const unsigned i : { 0u, 4u } ) {
                exec( ts, i ).run();
                EXPECT_EQ( counter, 1 );
                counter = 0;
        }
}

TEST( joque, filter )
{

        std::vector< int > sequence( 2 );
        std::iota( sequence.begin(), sequence.end(), 0 );
        std::vector< int > result;
        std::mutex         w_m;

        task_set ts{};
        ts.tasks["unwanted_test"] = task{ .job = [&]( const task& ) -> run_result {
                ADD_FAILURE();
                return { 0 };
        } };
        for ( const int i : sequence ) {
                ts.tasks["my_test_" + std::to_string( i )] = task{
                    .job = [&, i = i]( const task& ) -> run_result {
                            const std::lock_guard _{ w_m };
                            result.push_back( i );
                            return { 0 };
                    },
                    .run_after = { ts.tasks["unwanted_test"] },
                };
        }

        for ( const unsigned i : { 0u, 4u } ) {
                result.clear();
                exec( ts, i, "//my_test" ).run();
                std::sort( result.begin(), result.end() );

                EXPECT_EQ( sequence, result ) << "thread count: " << i;
        }
}

TEST( joque, failed_dep )
{

        task_set ts{};
        ts.tasks["my_test_a"] = task{
            .job = [&]( const task& ) -> run_result {
                    return { 1 };
            },
        };

        std::atomic< bool > v = false;
        ts.tasks["my_test_b"] = task{
            .job = [&]( const task& ) -> run_result {
                    v = true;
                    return { 0 };
            },
            .depends_on = { ts.tasks["my_test_a"] },
        };
        for ( const unsigned i : { 0u, 4u } ) {
                exec( ts, i ).run();

                EXPECT_FALSE( v );
        }
}

struct test_job
{
        bool is_invalidated = false;
};

template <>
struct job_traits< test_job >
{
        static inval_result is_invalidated( const test_job& j )
        {
                return { j.is_invalidated, "" };
        }

        static run_result run( const task&, test_job& )
        {
                return { 0 };
        }
};

TEST( joque, cyclic_invalidation )
{

        for ( bool is_invalid : { true, false } ) {
                task t{ .job = test_job{ is_invalid } };
                task t2{ .job = test_job{ false } };

                dag   g;
                auto& n1 = g.emplace( "//test1", t );
                auto& n2 = g.emplace( "//test2", t2 );
                auto& n3 = g.emplace( "//test3", t2 );

                add_edge( n1, n2, ekind::INVALIDATED_BY );
                add_edge( n2, n3, ekind::INVALIDATED_BY );
                add_edge( n3, n1, ekind::INVALIDATED_BY );

                auto rec = exec( std::move( g ), 0 ).run();
                EXPECT_EQ( rec->total_count, 3 );
                EXPECT_EQ( rec->stats[run_status::SKIP], is_invalid ? 0 : 3 );
        }
}

void normalize_vec( auto& vec )
{
        std::ranges::rotate( vec, std::ranges::min_element( vec ) );
}

struct cycle_test_vis : exec_visitor
{
        std::vector< const dag_node* > cycle;

        void on_detected_cycle( std::span< const dag_node* > cyc ) override
        {
                cycle.insert( cycle.end(), cyc.begin(), cyc.end() );
                normalize_vec( cycle );
        }
};

TEST( joque, cyclic_after )
{
        task t1{ .job = test_job{} };
        task t2{ .job = test_job{} };
        task t3{ .job = test_job{} };

        dag   g;
        auto& n1 = g.emplace( "//test1", t1 );
        auto& n2 = g.emplace( "//test2", t2 );
        auto& n3 = g.emplace( "//test3", t3 );

        add_edge( n1, n2, ekind::AFTER );
        add_edge( n2, n3, ekind::AFTER );
        add_edge( n3, n1, ekind::AFTER );

        cycle_test_vis                 vis;
        std::vector< const dag_node* > expected = { &n1, &n2, &n3 };
        normalize_vec( expected );

        try {
                auto rec = exec( std::move( g ), 0, vis ).run();
        }
        catch ( ... ) {
        }
        EXPECT_EQ( vis.cycle, expected );
}

}  // namespace joque
