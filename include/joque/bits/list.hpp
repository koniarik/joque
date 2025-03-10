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

#include "joque/bits/list_ptr.hpp"

#include <cstddef>
#include <type_traits>
#include <utility>

namespace joque::bits
{

template < typename Node, typename Accessor >
struct list_header;

/// Header accessor is a type that has static member function `get(n)` which
/// returns reference to list header stored in node `n`. Function should exist
/// for `n` being const or non-const.
template < typename T, typename Node >
concept header_accessor = requires( Node& n, const Node& cn ) {
        { T::get( n ) } -> std::convertible_to< list_header< Node, T >& >;
        { T::get( cn ) } -> std::convertible_to< const list_header< Node, T >& >;
};

// \brief List header (next/prev pointer) for double linked list node.
//
// Stores next/prev pointers for list node. Models a pattern where header stores
// pointers to the type of the node and has static accessor to get appropiate
// header from next/prev node. This makes it possible to have multiple different
// headers per node. (And hence each node can be part of multiple linked lists)
//
// \tparam Node type of nodes in the linked list
// \tparam Accessor
template < typename Node, typename Accessor >
struct list_header
{

        using node_type     = Node;
        using accessor_type = Accessor;
        using ptr_type      = list_ptr< Node, list_header, Accessor >;

        ptr_type next = nullptr;
        ptr_type prev = nullptr;

        list_header() = default;

        list_header( const list_header& )            = delete;
        list_header& operator=( const list_header& ) = delete;

        list_header( list_header&& other ) noexcept
        {
                *this = std::move( other );
        }

        list_header& operator=( list_header&& other ) noexcept
        {
                if ( this == &other )
                        return *this;

                next = std::exchange( other.next, nullptr );
                prev = std::exchange( other.prev, nullptr );
                if ( auto* h = next.find_header() )
                        h->prev = this;
                if ( auto* h = prev.find_header() )
                        h->next = next;

                return *this;
        }

        ~list_header();
};

template < typename Accessor, typename Node >
void list_unlink( Node& node );

template < typename Ptr, typename... Args >
auto& list_emplace_next( Ptr ptr, Args&&... args );

template < typename Ptr, typename Node >
void list_link_next( Ptr ptr, Node& next );

template < typename Header >
void list_delete_all_next( Header& header );

template < typename ListHeader >
class list_iterator
{
public:
        static constexpr bool is_const = std::is_const_v< ListHeader >;

        using header_type     = ListHeader;
        using difference_type = std::ptrdiff_t;
        using value_type      = std::conditional_t<
                 is_const,
                 const typename header_type::node_type,
                 typename header_type::node_type >;
        using accessor_type = typename header_type::accessor_type;

        list_iterator() = default;

        list_iterator( value_type* node );

        value_type& operator*();

        value_type& operator*() const;

        value_type* operator->();

        value_type* operator->() const;

        list_iterator& operator++();

        list_iterator& operator--();

        list_iterator operator++( int );

        list_iterator operator--( int );

        auto operator<=>( const list_iterator& ) const = default;

private:
        header_type& list_header()
        {
                return accessor_type::get( *node_ );
        }

        value_type* node_ = nullptr;
};

template < typename ListHeader >
class list
{
public:
        using header_type    = ListHeader;
        using node_type      = typename header_type::node_type;
        using accessor_type  = typename header_type::accessor_type;
        using iterator       = list_iterator< header_type >;
        using const_iterator = list_iterator< const header_type >;
        using ptr_type       = list_ptr< node_type, header_type, accessor_type >;

        list()                         = default;
        list( const list& )            = delete;
        list( list&& )                 = default;
        list& operator=( const list& ) = delete;
        list& operator=( list&& )      = default;

        [[nodiscard]] iterator       begin();
        [[nodiscard]] const_iterator begin() const;

        [[nodiscard]] iterator       end();
        [[nodiscard]] const_iterator end() const;

        [[nodiscard]] node_type& front()
        {
                return *begin();
        }

        [[nodiscard]] const node_type& front() const
        {
                return *begin();
        };

        [[nodiscard]] node_type& back()
        {
                return *++end();
        }

        [[nodiscard]] const node_type& back() const
        {
                return *++end();
        };

        template < typename... Args >
        node_type& emplace_front( Args&&... args );

        void link_front( node_type& node );

        [[nodiscard]] bool empty() const;

        void clear_if( auto&& f );

        ~list();

private:
        header_type header_;
};

template < typename Node, typename Accessor >
list_header< Node, Accessor >::~list_header()
{
        static_assert( header_accessor< Accessor, Node > );

        if ( auto* h = next.find_header() )
                h->prev = prev;

        if ( auto* h = prev.find_header() )
                h->next = next;
}

template < typename Accessor, typename Node >
void list_unlink( Node& node )
{
        auto& lnode = Accessor::get( node );

        if ( lnode.next != nullptr )
                Accessor::get( *lnode.next ).prev = lnode.prev;
        if ( lnode.prev != nullptr )
                Accessor::get( *lnode.prev ).next = lnode.next;

        lnode.prev = nullptr;
        lnode.next = nullptr;
}

template < typename Ptr, typename... Args >
auto& list_emplace_next( Ptr ptr, Args&&... args )
{
        using Node = typename Ptr::node_type;

        Node* nnode = new Node{ std::forward< Args >( args )... };
        list_link_next( ptr, *nnode );
        return *nnode;
}

template < typename Ptr, typename Node >
void list_link_next( Ptr ptr, Node& next )
{
        static_assert( std::same_as< typename Ptr::node_type, Node > );
        using Accessor = typename Ptr::accessor_type;
        using Header   = typename Ptr::header_type;

        Header& nnode = Accessor::get( next );

        nnode.next = ptr.find_header()->next;
        if ( nnode.next != nullptr )
                nnode.next.find_header()->prev = &next;

        ptr.find_header()->next = &next;
        nnode.prev              = ptr;
}

template < typename Header >
void list_delete_all_next( Header& header )
{
        while ( auto* node = header.next.get_node() )
                delete node;
}

template < typename ListHeader >
list_iterator< ListHeader >::list_iterator( value_type* node )
  : node_( node )
{
}

template < typename ListHeader >
list_iterator< ListHeader >::value_type& list_iterator< ListHeader >::operator*()
{
        return *node_;
}

template < typename ListHeader >
list_iterator< ListHeader >::value_type& list_iterator< ListHeader >::operator*() const
{
        return *node_;
}

template < typename ListHeader >
list_iterator< ListHeader >::value_type* list_iterator< ListHeader >::operator->()
{
        return node_;
}

template < typename ListHeader >
list_iterator< ListHeader >::value_type* list_iterator< ListHeader >::operator->() const
{
        return node_;
}

template < typename ListHeader >
list_iterator< ListHeader >& list_iterator< ListHeader >::operator++()
{
        node_ = list_header().next.get_node();
        return *this;
}

template < typename ListHeader >
list_iterator< ListHeader >& list_iterator< ListHeader >::operator--()
{
        node_ = list_header().prev.get_node();
        return *this;
}

template < typename ListHeader >
list_iterator< ListHeader > list_iterator< ListHeader >::operator++( int )
{
        list_iterator res{ *this };
        ++( *this );
        return res;
}

template < typename ListHeader >
list_iterator< ListHeader > list_iterator< ListHeader >::operator--( int )
{
        list_iterator res{ *this };
        --( *this );
        return res;
}

template < typename ListHeader >
list< ListHeader >::iterator list< ListHeader >::begin()
{
        iterator res{ header_.next.get_node() };
        return res;
}

template < typename ListHeader >
list< ListHeader >::const_iterator list< ListHeader >::begin() const
{
        const_iterator res{ header_.next.get_node() };
        return res;
}

template < typename ListHeader >
list< ListHeader >::iterator list< ListHeader >::end()
{
        return iterator{ nullptr };
}

template < typename ListHeader >
list< ListHeader >::const_iterator list< ListHeader >::end() const
{
        return const_iterator{ nullptr };
}

template < typename ListHeader >
template < typename... Args >
list< ListHeader >::node_type& list< ListHeader >::emplace_front( Args&&... args )
{
        return list_emplace_next( ptr_type{ &header_ }, std::forward< Args >( args )... );
}

template < typename ListHeader >
void list< ListHeader >::link_front( node_type& node )
{
        list_link_next( ptr_type{ &header_ }, node );
}

template < typename ListHeader >
bool list< ListHeader >::empty() const
{
        return header_.next == nullptr;
}

template < typename ListHeader >
void list< ListHeader >::clear_if( auto&& f )
{
        for ( auto iter = begin(); iter != end(); ) {
                auto* node = &*( iter++ );
                if ( f( *node ) )
                        delete node;
        }
}

template < typename ListHeader >
list< ListHeader >::~list()
{
        list_delete_all_next( header_ );
}

}  // namespace joque::bits
