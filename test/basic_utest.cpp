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

}  // namespace joque
