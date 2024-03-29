#include "joque/format.hpp"

#include <gtest/gtest.h>

namespace joque
{

auto out( std::string_view s )
{
        return std::list{ output_chunk{ output_chunk::STANDARD, std::string{ s } } };
}

TEST( joq, fmt_nested )
{
        std::stringstream ss;

        format_nested( ss, "x", {} );
        EXPECT_TRUE( ss.view().empty() );

        format_nested( ss, "x", out( "y" ) );
        EXPECT_EQ( ss.view(), "xy\n" );

        ss = std::stringstream{};
        format_nested( ss, "x", out( "y\n" ) );
        EXPECT_EQ( ss.view(), "xy\n" );

        ss = std::stringstream{};
        format_nested( ss, "x", out( "y\nz\n" ) );
        EXPECT_EQ( ss.view(), "xy\nxz\n" );
}

}  // namespace joque
