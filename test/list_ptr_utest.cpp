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

using tlist   = list< list_header< tnode, taccessor > >;
using tptr    = typename tlist::ptr_type;
using theader = typename tlist::header_type;

TEST( list_header, empty )
{
        tptr        p;
        const tptr& cp = p;

        EXPECT_EQ( cp.type(), tptr::mark::NULL_TYPE );
        EXPECT_EQ( p.get< tnode >(), nullptr );
        EXPECT_EQ( cp.get< tnode >(), nullptr );
        EXPECT_EQ( p.get< theader >(), nullptr );
        EXPECT_EQ( cp.get< theader >(), nullptr );
        EXPECT_EQ( p.get_node(), nullptr );
        EXPECT_EQ( cp.get_node(), nullptr );
        EXPECT_EQ( p.find_header(), nullptr );
        EXPECT_EQ( cp.find_header(), nullptr );

        p.match(
            []( tnode& ) {
                    FAIL();
            },
            []( theader& ) {
                    FAIL();
            } );
        cp.match(
            []( const tnode& ) {
                    FAIL();
            },
            []( const theader& ) {
                    FAIL();
            } );
}

TEST( list_header, node )
{
        tnode n;

        tptr        p{ &n };
        const tptr& cp = p;

        EXPECT_EQ( cp.type(), tptr::mark::NODE_TYPE );
        EXPECT_EQ( p.get< tnode >(), &n );
        EXPECT_EQ( cp.get< tnode >(), &n );
        EXPECT_EQ( p.get< theader >(), nullptr );
        EXPECT_EQ( cp.get< theader >(), nullptr );
        EXPECT_EQ( p.get_node(), &n );
        EXPECT_EQ( cp.get_node(), &n );
        EXPECT_EQ( p.find_header(), &n.lheader );
        EXPECT_EQ( cp.find_header(), &n.lheader );

        std::size_t node_counter = 0;
        p.match(
            [&]( tnode& ) {
                    node_counter += 1;
            },
            []( theader& ) {
                    FAIL();
            } );
        EXPECT_EQ( node_counter, 1 );

        cp.match(
            [&]( const tnode& ) {
                    node_counter += 1;
            },
            []( const theader& ) {
                    FAIL();
            } );
        EXPECT_EQ( node_counter, 2 );
}

TEST( list_header, header )
{
        theader h;

        tptr        p{ &h };
        const tptr& cp = p;

        EXPECT_EQ( cp.type(), tptr::mark::HEADER_TYPE );
        EXPECT_EQ( p.get< tnode >(), nullptr );
        EXPECT_EQ( cp.get< tnode >(), nullptr );
        EXPECT_EQ( p.get< theader >(), &h );
        EXPECT_EQ( cp.get< theader >(), &h );
        EXPECT_EQ( p.get_node(), nullptr );
        EXPECT_EQ( cp.get_node(), nullptr );
        EXPECT_EQ( p.find_header(), &h );
        EXPECT_EQ( cp.find_header(), &h );

        std::size_t node_counter = 0;
        p.match(
            []( tnode& ) {
                    FAIL();
            },
            [&]( theader& ) {
                    node_counter += 1;
            } );
        EXPECT_EQ( node_counter, 1 );

        cp.match(
            []( const tnode& ) {
                    FAIL();
            },
            [&]( const theader& ) {
                    node_counter += 1;
            } );
        EXPECT_EQ( node_counter, 2 );
}

}  // namespace joque::bits
