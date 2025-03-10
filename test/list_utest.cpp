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
namespace
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
}  // namespace

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
