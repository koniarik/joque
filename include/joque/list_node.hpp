#pragma once

#include <cstddef>
#include <type_traits>
#include <utility>

namespace joque
{

template < typename Node, typename Accessor >
struct list_node
{
        using node_type     = Node;
        using accessor_type = Accessor;

        Node* next = nullptr;
        Node* prev = nullptr;
};

template < typename Accessor, typename Node, typename... Args >
void list_emplace_next( Node& node, Args&&... args )
{
        Node* nnode = new Node{ std::forward< Args >( args )... };
        auto& lnode = Accessor::get( *nnode );
        auto& fnode = Accessor::get( node );

        lnode.next = fnode.next;
        if ( lnode.next != nullptr ) {
                Accessor::get( *lnode.next ).prev = nnode;
        }

        fnode.next = nnode;
        lnode.prev = &node;
}

template < typename Accessor, typename Node >
void list_unlink( Node& node )
{
        auto& lnode = Accessor::get( node );

        if ( lnode.next != nullptr ) {
                Accessor::get( *lnode.next ).prev = lnode.prev;
        }
        if ( lnode.prev != nullptr ) {
                Accessor::get( *lnode.prev ).next = lnode.next;
        }

        lnode.prev = nullptr;
        lnode.next = nullptr;
}

template < typename Accessor, typename Node >
void list_delete_all_next( Node& node )
{
        auto& lnode = Accessor::get( node );
        while ( lnode.next != nullptr ) {
                auto* next = lnode.next;
                list_unlink< Accessor >( *next );
                delete next;
        }
}

template < typename ListNodeType >
class list_iterator
{
public:
        static constexpr bool is_const = std::is_const_v< ListNodeType >;

        using list_node_type  = ListNodeType;
        using difference_type = std::ptrdiff_t;
        using value_type      = std::conditional_t<
            is_const,
            const typename list_node_type::node_type,
            typename list_node_type::node_type >;
        using accessor_type = typename list_node_type::accessor_type;

        list_iterator() = default;

        list_iterator( value_type* node )
          : node( node )
        {
        }

        value_type& operator*()
        {
                return *node;
        }

        value_type& operator*() const
        {
                return *node;
        }

        value_type* operator->()
        {
                return node;
        }

        value_type* operator->() const
        {
                return node;
        }

        list_iterator& operator++()
        {
                node = list_node().next;
                return *this;
        }
        list_iterator& operator--()
        {
                node = list_node().prev;
                return *this;
        }
        list_iterator operator++( int )
        {
                list_iterator res{ *this };
                ++( *this );
                return res;
        }
        list_iterator operator--( int )
        {
                list_iterator res{ *this };
                --( *this );
                return res;
        }

private:
        list_node_type& list_node()
        {
                return accessor_type::get( *node );
        }

        value_type* node = nullptr;
};

template < typename ListNodeType >
bool operator==( const list_iterator< ListNodeType >& lh, const list_iterator< ListNodeType >& rh )
{
        return &*lh == &*rh;
}

template < typename ListNodeType >
class list
{
public:
        using list_node_type = ListNodeType;
        using node_type      = typename list_node_type::node_type;
        using accessor_type  = typename list_node_type::accessor_type;
        using iterator       = list_iterator< list_node_type >;
        using const_iterator = list_iterator< const list_node_type >;

        iterator begin()
        {
                iterator res{ &first };
                ++res;
                return res;
        }

        const_iterator begin() const
        {
                const_iterator res{ &first };
                ++res;
                return res;
        }

        iterator end()
        {
                return iterator{ nullptr };
        }

        const_iterator end() const
        {
                return const_iterator{ nullptr };
        }

        template < typename... Args >
        void emplace_front( Args&&... args )
        {
                list_emplace_next< accessor_type >( first, std::forward< Args >( args )... );
        }

        bool empty() const
        {
                return list_node( first ).next == nullptr;
        }

        ~list()
        {
                list_delete_all_next< accessor_type >( first );
        }

private:
        static auto& list_node( auto& node )
        {
                return accessor_type::get( node );
        }

        node_type first;
};

}  // namespace joque
