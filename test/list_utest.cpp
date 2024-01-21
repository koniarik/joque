
#include "joque/list_node.hpp"

#include <gtest/gtest.h>
#include <iterator>

namespace joque
{

struct node_accessor;

struct node
{
        int i;

        list_node< node, node_accessor > lnode = {};
};

struct node_accessor
{
        static auto& get( auto& n )
        {
                return n.lnode;
        }
};

using titer = list_iterator< list_node< node, node_accessor > >;
using tlist = list< list_node< node, node_accessor > >;

static_assert( std::bidirectional_iterator< titer > );

TEST( list_node, empty )
{
        tlist l{};

        EXPECT_TRUE( l.empty() );
        EXPECT_EQ( l.begin(), l.end() );
}

TEST( list_node, sizeone )
{
        tlist l{};
        l.emplace_front( 42 );

        EXPECT_FALSE( l.empty() );
        EXPECT_NE( l.begin(), l.end() );
        EXPECT_EQ( l.begin()->i, 42 );
}

TEST( list_node, multiple )
{
        tlist l{};
        l.emplace_front( 3 );
        l.emplace_front( 2 );
        l.emplace_front( 1 );

        int i = 0;
        for ( node& n : l ) {
                i += 1;
                EXPECT_EQ( n.i, i );
        }
        EXPECT_EQ( i, 3 );
}

}  // namespace joque
