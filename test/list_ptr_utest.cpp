/// MIT License
///
/// Copyright (c) 2025 Jan Veverak Koniarik
///
/// Permission is hereby granted, free of charge, to any person obtaining a copy
/// of this software and associated documentation files (the "Software"), to deal
/// in the Software without restriction, including without limitation the rights
/// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
/// copies of the Software, and to permit persons to whom the Software is
/// furnished to do so, subject to the following conditions:
///
/// The above copyright notice and this permission notice shall be included in all
/// copies or substantial portions of the Software.
///
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
/// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
/// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
/// SOFTWARE.
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
        EXPECT_EQ( p, nullptr );
        EXPECT_EQ( cp, nullptr );

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

        {
                tnode       n;
                tptr        np{ &n };
                const tptr& cnp = np;

                EXPECT_NE( p, np );
                EXPECT_NE( cp, np );
                EXPECT_NE( p, cnp );
                EXPECT_NE( cp, cnp );
        }

        {
                theader     h;
                tptr        hp{ &h };
                const tptr& chp = hp;

                EXPECT_NE( p, hp );
                EXPECT_NE( cp, hp );
                EXPECT_NE( p, chp );
                EXPECT_NE( cp, chp );
        }
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

        {
                theader     h;
                tptr        hp{ &h };
                const tptr& chp = hp;

                EXPECT_NE( p, hp );
                EXPECT_NE( cp, hp );
                EXPECT_NE( p, chp );
                EXPECT_NE( cp, chp );
        }
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
