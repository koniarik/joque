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
#pragma once

#include "joque/bits/list.hpp"

namespace joque::bits
{

struct lheader_accessor
{
        static auto& get( auto& n )
        {
                return n.lheader_;
        }
};

struct lrheader_accessor
{
        static auto& get( auto& n )
        {
                return n.lrheader_;
        }
};

template < typename T >
class gedge
{
public:
        using lheader_type  = list_header< gedge, lheader_accessor >;
        using lrheader_type = list_header< gedge, lrheader_accessor >;
        using list_type     = list< lheader_type >;
        using rlist_type    = list< lrheader_type >;

        gedge( const gedge& other )            = delete;
        gedge( gedge&& other )                 = delete;
        gedge& operator=( const gedge& other ) = delete;
        gedge& operator=( gedge&& other )      = delete;

        template < typename... Args >
        gedge( Args&&... args )
          : content_( std::forward< Args >( args )... )
        {
        }

        T* operator->()
        {
                return &content_;
        }

        const T* operator->() const
        {
                return &content_;
        }

        T& operator*()
        {
                return content_;
        }

        const T& operator*() const
        {
                return content_;
        }

private:
        friend lheader_accessor;
        friend lrheader_accessor;

        T content_;

        lheader_type  lheader_;
        lrheader_type lrheader_;
};

template < typename T, typename EdgeType >
class gnode
{
public:
        using edge_type    = EdgeType;
        using lheader_type = list_header< gnode, lheader_accessor >;
        using edge_list    = typename edge_type::list_type;
        using edge_rlist   = typename edge_type::rlist_type;

        using list_type = list< lheader_type >;

        gnode() = default;

        gnode( const gnode& other )            = delete;
        gnode( gnode&& other )                 = delete;
        gnode& operator=( const gnode& other ) = delete;
        gnode& operator=( gnode&& other )      = delete;

        template < typename... Args >
        gnode( Args&&... args )
          : content_( std::forward< Args >( args )... )
        {
        }

        T* operator->()
        {
                return &content_;
        }

        const T* operator->() const
        {
                return &content_;
        }

        T& operator*()
        {
                return content_;
        }

        const T& operator*() const
        {
                return content_;
        }

        [[nodiscard]] edge_list& out_edges()
        {
                return out_edges_;
        }

        [[nodiscard]] const edge_list& out_edges() const
        {
                return out_edges_;
        }

        [[nodiscard]] edge_rlist& in_edges()
        {
                return in_edges_;
        }

        [[nodiscard]] const edge_rlist& in_edges() const
        {
                return in_edges_;
        }

private:
        friend lheader_accessor;

        T content_;

        edge_list  out_edges_;
        edge_rlist in_edges_;

        lheader_type lheader_;
};

template < typename NodeType >
class graph
{
public:
        using node_type = NodeType;
        using edge_type = typename node_type::edge_type;
        using node_list = typename node_type::list_type;

        constexpr graph() noexcept             = default;
        graph( const graph& other )            = delete;
        graph( graph&& other )                 = default;
        graph& operator=( const graph& other ) = delete;
        graph& operator=( graph&& other )      = default;

        auto begin()
        {
                return nodes_.begin();
        }

        [[nodiscard]] auto begin() const
        {
                return nodes_.begin();
        }

        auto end()
        {
                return nodes_.end();
        }

        [[nodiscard]] auto end() const
        {
                return nodes_.end();
        }

        void clear_if( auto&& f )
        {
                nodes_.clear_if( f );
        }

        template < typename... Ts >
        node_type& emplace( Ts&&... args )
        {
                return nodes_.emplace_front( std::forward< Ts >( args )... );
        }

private:
        node_list nodes_;
};

}  // namespace joque::bits
