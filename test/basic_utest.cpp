#include "joque/exec.hpp"
#include "joque/task.hpp"
#include "joque/traits.hpp"

#include <gtest/gtest.h>
#include <numeric>
#include <ranges>

namespace joque
{

TEST( joque, compile_test )
{
        resource my_dev{ .name = "my_device" };
        task_set ts{};

        task_ptr& t1 =
            ( ts.tasks["my_test"] = task{
                  .job       = process::derive( "ls", "./", out( "/tmp/out.txt" ) ),
                  .resources = { my_dev },
              } );

        ts.tasks["my_test2"] = task{
            .job = []( const task_iface& ) -> run_result {
                    return { 0 };
            },
            .depends_on = { *t1 },
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
        for ( int i : sequence ) {
                ts.tasks["my_test_" + std::to_string( i )] = task{
                    .job = [&, i = i]( const task_iface& ) -> run_result {
                            std::lock_guard _{ w_m };
                            result.push_back( i );
                            return { 0 };
                    },
                };
        }

        for ( unsigned i : { 0u, 4u } ) {
                result.clear();
                exec( ts, i ).run();
                std::sort( result.begin(), result.end() );

                EXPECT_EQ( sequence, result ) << "thread count: " << i;
        }
}

TEST( joque, dep )
{
        task_set                      ts{};
        std::mutex                    m;
        std::set< const task_iface* > finished;

        auto f = [&]( const task_iface& t ) -> run_result {
                std::lock_guard _{ m };
                for ( const task_iface& dep : t.depends_on() )
                        EXPECT_TRUE( finished.contains( &dep ) );
                finished.insert( &t );
                return { 0 };
        };

        const task_ptr* last =
            &( ts.tasks["my_test"] = task{
                   .job = f,
               } );
        for ( std::size_t i : std::views::iota( 0u, 10u ) ) {
                ts.tasks["my_test_" + std::to_string( i ) + "_a"] = task{
                    .job        = f,
                    .depends_on = { **last },
                };
                last =
                    &( ts.tasks["my_test_" + std::to_string( i ) + "_b"] = task{
                           .job        = f,
                           .depends_on = { **last },
                       } );
        }

        for ( unsigned i : { 0u, 4u } ) {
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
        ts.tasks["unwanted_test"] = task{ .job = [&]( const task_iface& ) -> run_result {
                ADD_FAILURE();
                return { 0 };
        } };
        for ( int i : sequence ) {
                ts.tasks["my_test_" + std::to_string( i )] = task{
                    .job = [&, i = i]( const task_iface& ) -> run_result {
                            std::lock_guard _{ w_m };
                            result.push_back( i );
                            return { 0 };
                    },
                    .run_after = { *ts.tasks["unwanted_test"] },
                };
        }


        for ( unsigned i : { 0u, 4u } ) {
                result.clear();
                exec( ts, i, "//my_test" ).run();
                std::sort( result.begin(), result.end() );

                EXPECT_EQ( sequence, result ) << "thread count: " << i;
        }
}

}  // namespace joque
