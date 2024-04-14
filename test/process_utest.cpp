#include "joque/exec.hpp"
#include "joque/exec_coro.hpp"
#include "joque/out.hpp"
#include "joque/process.hpp"
#include "joque/records.hpp"
#include "joque/task.hpp"

#include <chrono>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <thread>

namespace joque
{

class joque_fixture : public ::testing::Test
{
public:
        std::filesystem::path f1_name = std::tmpnam( nullptr );
        std::filesystem::path f2_name = std::tmpnam( nullptr );

        ~joque_fixture()
        {
                std::remove( f1_name.c_str() );
                std::remove( f2_name.c_str() );
        }
};

TEST_F( joque_fixture, process_no_update )
{

        {
                std::ofstream os{ f1_name };
                os << "test";
        }

        std::this_thread::sleep_for( std::chrono::milliseconds( 50 ) );

        {
                std::ofstream os{ f2_name };
        }

        task_set ts;

        ts.tasks["p"] = task{ .job = process::derive( "cp", f1_name, out( f2_name ) ) };

        EXPECT_LT( last_write_time( f1_name ), last_write_time( f2_name ) );

        // f2 should no be overwritten by f1 as f2 should be newer than f1
        exec_coro                    c   = exec( ts );
        std::optional< exec_record > res = c.run();
        EXPECT_TRUE( res.has_value() );
        EXPECT_EQ( res->stats[run_status::SKIP], 1 );
        EXPECT_EQ( res->stats[run_status::FAIL], 0 );
        EXPECT_LT( last_write_time( f1_name ), last_write_time( f2_name ) );
}

TEST_F( joque_fixture, update )
{

        {
                std::ofstream os{ f2_name };
        }

        std::this_thread::sleep_for( std::chrono::milliseconds( 50 ) );

        {
                std::ofstream os{ f1_name };
                os << "test";
        }

        task_set ts;

        ts.tasks["p"] = task{ .job = process::derive( "cp", f1_name, out( f2_name ) ) };

        EXPECT_LT( last_write_time( f2_name ), last_write_time( f1_name ) );

        // f2 should be overwritten by f1 as f2 should be older than f1
        exec_coro                    c   = exec( ts );
        std::optional< exec_record > res = c.run();
        EXPECT_TRUE( res.has_value() );
        EXPECT_EQ( res->stats[run_status::SKIP], 0 );
        EXPECT_EQ( res->stats[run_status::FAIL], 0 );

        std::this_thread::sleep_for( std::chrono::milliseconds( 50 ) );

        EXPECT_LT( last_write_time( f1_name ), last_write_time( f2_name ) );
}

TEST( joque, corner_cases )
{
        process p;
        p.input.push_back( "/tmp/nothing.txt" );

        // if no output is given, is invalidated is always true
        EXPECT_TRUE( p.output.empty() );
        EXPECT_FALSE( p.input.empty() );
        EXPECT_TRUE( job_traits< process >::is_invalidated( p ).invalidated );

        p.input.clear();
        p.output.push_back( "/tmp/nothing.txt" );

        // if no input is given, is invalidated is true if file does not exists
        EXPECT_FALSE( p.output.empty() );
        EXPECT_TRUE( p.input.empty() );
        EXPECT_FALSE( job_traits< process >::is_invalidated( p ).invalidated );
}

}  // namespace joque
