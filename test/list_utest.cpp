#include "joque/bits/list.hpp"

#include <gtest/gtest.h>

namespace joque::bits
{

struct taccessor;

struct tnode
{
        int i;

        list_header< tnode, taccessor > lheader = {};
};

struct taccessor
{
        static auto& get( auto& n )
        {
                return n.lheader;
        }
};

using titer = list_iterator< list_header< tnode, taccessor > >;
using tlist = list< list_header< tnode, taccessor > >;

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
        for ( const tnode& n : l ) {
                i += 1;
                EXPECT_EQ( n.i, i );
        }
        EXPECT_EQ( i, 3 );
}

}  // namespace joque::bits
