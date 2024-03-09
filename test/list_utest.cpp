#include "joque/bits/list.hpp"

#include <gtest/gtest.h>

namespace joque::bits
{

struct node_accessor;

struct node
{
        int i;

        list_header< node, node_accessor > lheader = {};
};

struct node_accessor
{
        static auto& get( auto& n )
        {
                return n.lheader;
        }
};

using titer = list_iterator< list_header< node, node_accessor > >;
using tlist = list< list_header< node, node_accessor > >;

static_assert( std::bidirectional_iterator< titer > );

TEST( list_header, empty )
{
        tlist l{};

        EXPECT_TRUE( l.empty() );
        EXPECT_EQ( l.begin(), l.end() );
}

TEST( list_header, sizeone )
{
        tlist l{};
        l.emplace_front( 42 );

        EXPECT_FALSE( l.empty() );
        EXPECT_NE( l.begin(), l.end() );
        EXPECT_EQ( l.begin()->i, 42 );
}

TEST( list_header, multiple )
{
        tlist l{};
        l.emplace_front( 3 );
        l.emplace_front( 2 );
        l.emplace_front( 1 );

        int i = 0;
        for ( const node& n : l ) {
                i += 1;
                EXPECT_EQ( n.i, i );
        }
        EXPECT_EQ( i, 3 );
}

}  // namespace joque::bits
