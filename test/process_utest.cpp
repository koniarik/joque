#include "joque/exec.hpp"
#include "joque/process.hpp"
#include "joque/task.hpp"

#include <fstream>
#include <gtest/gtest.h>

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

        {
                std::ofstream os{ f2_name };
        }

        task_set ts;

        ts.tasks["p"] = task{ .job = process::derive( "cp", f1_name, out( f2_name ) ) };

        EXPECT_LT( last_write_time( f1_name ), last_write_time( f2_name ) );

        // f2 should no be overwritten by f1 as f2 should be newer than f1
        exec( ts ).run();

        EXPECT_LT( last_write_time( f1_name ), last_write_time( f2_name ) );
}

TEST_F( joque_fixture, update )
{

        {
                std::ofstream os{ f2_name };
        }

        {
                std::ofstream os{ f1_name };
                os << "test";
        }

        task_set ts;

        ts.tasks["p"] = task{ .job = process::derive( "cp", f1_name, out( f2_name ) ) };

        EXPECT_LT( last_write_time( f2_name ), last_write_time( f1_name ) );

        // f2 should be overwritten by f1 as f2 should be older than f1
        exec( ts ).run();

        EXPECT_LT( last_write_time( f1_name ), last_write_time( f2_name ) );
}

}  // namespace joque
