#include "joque/exec.hpp"
#include "joque/jexcp.hpp"
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
        static bool is_invalidated( const test_job& j )
        {
                return j.is_invalidated;
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
                EXPECT_EQ( rec->skipped_count, is_invalid ? 0 : 3 );
        }
}

std::vector< const task* > normalize_vec( std::vector< const task* > vec )
{
        std::vector< const task* > res;

        auto pivot = std::ranges::min_element( vec );
        res.insert( res.end(), pivot, vec.end() );
        res.insert( res.end(), vec.begin(), pivot );
        return res;
}

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

        std::vector< const task* > expected = { &t1, &t2, &t3 };

        try {
                auto rec = exec( std::move( g ), 0 ).run();
                FAIL();
        }
        catch ( cycle_excp& e ) {
                EXPECT_EQ( normalize_vec( e.cycle ), normalize_vec( expected ) );
        }
}

}  // namespace joque
