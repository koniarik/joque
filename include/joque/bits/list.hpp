#pragma once

#include <concepts>
#include <cstddef>
#include <memory>
#include <type_traits>
#include <utility>

namespace joque::bits
{

template < typename Node, typename Accessor >
struct list_header;

/// Header accessor is a type that has static member function `get(n)` which returns reference to
/// list header stored in node `n`. Function should exist for `n` being const or non-const.
template < typename T, typename Node >
concept header_accessor = requires( Node& n, const Node& cn ) {
        {
                T::get( n )
        } -> std::convertible_to< list_header< Node, T >& >;
        {
                T::get( cn )
        } -> std::convertible_to< const list_header< Node, T >& >;
};


// \brief List header (next/prev pointer) for double linked list node.
//
// Stores next/prev pointers for list node. Models a pattern where header stores pointers to the
// type of the node and has static accessor to get appropiate header from next/prev node. This makes
// it possible to have multiple different headers per node. (And hence each node can be part of
// multiple linked lists)
//
// \tparam Node type of nodes in the linked list
// \tparam Accessor
template < typename Node, typename Accessor >
struct list_header
{

        using node_type     = Node;
        using accessor_type = Accessor;

        Node* next = nullptr;
        Node* prev = nullptr;

        list_header() = default;

        list_header( const list_header& )                      = delete;
        list_header& operator=( const list_header& )           = delete;
        list_header( list_header&& other ) noexcept            = delete;
        list_header& operator=( list_header&& other ) noexcept = delete;

        ~list_header();
};

template < typename Accessor, typename Node >
void list_unlink( Node& node );

template < typename Accessor, typename Node, typename... Args >
Node& list_emplace_next( Node& node, Args&&... args );

template < typename Accessor, typename Node >
void list_link_next( Node& current, Node& next );

template < typename Accessor, typename Node >
void list_delete_all_next( Node& node );

template < typename ListHeader >
class list_iterator
{
public:
        static constexpr bool is_const = std::is_const_v< ListHeader >;

        using list_header_type = ListHeader;
        using difference_type  = std::ptrdiff_t;
        using value_type       = std::conditional_t<
            is_const,
            const typename list_header_type::node_type,
            typename list_header_type::node_type >;
        using accessor_type = typename list_header_type::accessor_type;

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
        list_header_type& list_header()
        {
                return accessor_type::get( *node );
        }

        value_type* node = nullptr;
};

template < typename ListHeader >
class list
{
public:
        using list_header_type = ListHeader;
        using node_type        = typename list_header_type::node_type;
        using accessor_type    = typename list_header_type::accessor_type;
        using iterator         = list_iterator< list_header_type >;
        using const_iterator   = list_iterator< const list_header_type >;

        list() = default;

        iterator       begin();
        const_iterator begin() const;

        iterator       end();
        const_iterator end() const;

        template < typename... Args >
        node_type& emplace_front( Args&&... args );

        void link_front( node_type& node );

        [[nodiscard]] bool empty() const;

        void clear_if( auto&& f );

        ~list();

private:
        static auto& list_header( auto& node );

        node_type first_;
};


template < typename Node, typename Accessor >
list_header< Node, Accessor >::~list_header()
{
        static_assert( header_accessor< Accessor, Node > );

        if ( next != nullptr )
                Accessor::get( *next ).prev = prev;
        if ( prev != nullptr )
                Accessor::get( *prev ).next = next;
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

template < typename Accessor, typename Node, typename... Args >
Node& list_emplace_next( Node& node, Args&&... args )
{
        Node* nnode = new Node{ std::forward< Args >( args )... };
        list_link_next< Accessor >( node, *nnode );
        return *nnode;
}

template < typename Accessor, typename Node >
void list_link_next( Node& current, Node& next )
{
        auto& nnode = Accessor::get( next );
        auto& cnode = Accessor::get( current );

        nnode.next = cnode.next;
        if ( nnode.next != nullptr )
                Accessor::get( *nnode.next ).prev = &next;

        cnode.next = &next;
        nnode.prev = &current;
}

template < typename Accessor, typename Node >
void list_delete_all_next( Node& node )
{
        auto& lnode = Accessor::get( node );
        while ( lnode.next != nullptr )
                delete lnode.next;
}

template < typename ListHeader >
list_iterator< ListHeader >::list_iterator( value_type* node )
  : node( node )
{
}

template < typename ListHeader >
list_iterator< ListHeader >::value_type& list_iterator< ListHeader >::operator*()
{
        return *node;
}

template < typename ListHeader >
list_iterator< ListHeader >::value_type& list_iterator< ListHeader >::operator*() const
{
        return *node;
}

template < typename ListHeader >
list_iterator< ListHeader >::value_type* list_iterator< ListHeader >::operator->()
{
        return node;
}

template < typename ListHeader >
list_iterator< ListHeader >::value_type* list_iterator< ListHeader >::operator->() const
{
        return node;
}

template < typename ListHeader >
list_iterator< ListHeader >& list_iterator< ListHeader >::operator++()
{
        node = list_header().next;
        return *this;
}

template < typename ListHeader >
list_iterator< ListHeader >& list_iterator< ListHeader >::operator--()
{
        node = list_header().prev;
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
        iterator res{ list_header( first_ ).next };
        return res;
}

template < typename ListHeader >
list< ListHeader >::const_iterator list< ListHeader >::begin() const
{
        const_iterator res{ list_header( first_ ).next };
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
        return list_emplace_next< accessor_type >( first_, std::forward< Args >( args )... );
}

template < typename ListHeader >
void list< ListHeader >::link_front( node_type& node )
{
        list_link_next< accessor_type >( first_, node );
}

template < typename ListHeader >
bool list< ListHeader >::empty() const
{
        return list_header( first_ ).next == nullptr;
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
        list_delete_all_next< accessor_type >( first_ );
}

template < typename ListHeader >
auto& list< ListHeader >::list_header( auto& node )
{
        return accessor_type::get( node );
}


}  // namespace joque::bits
