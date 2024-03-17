#pragma once

#include <concepts>
#include <cstdint>


namespace joque::bits
{

template < typename Node, typename Header, typename Accessor >
class list_ptr
{
public:
        enum class mark : std::uintptr_t
        {
                NODE_TYPE   = 0x00,
                HEADER_TYPE = 0x01,
                NULL_TYPE   = 0x02
        };

        using node_type     = Node;
        using header_type   = Header;
        using accessor_type = Accessor;

        list_ptr() noexcept = default;

        list_ptr( std::nullptr_t ) noexcept
        {
        }

        list_ptr( Node* item ) noexcept
          : ptr_( item )
        {
        }

        list_ptr( Header* item ) noexcept
        {
                if ( item != nullptr ) {
                        auto raw = reinterpret_cast< std::uintptr_t >( item ) | 0x01;
                        ptr_     = reinterpret_cast< void* >( raw );
                }
        }

        mark type() const
        {
                static_assert( alignof( Node ) > 1 );
                static_assert( alignof( Header ) > 1 );
                if ( ptr_ == nullptr )
                        return mark::NULL_TYPE;
                if ( ctlbit() )
                        return mark::HEADER_TYPE;
                return mark::NODE_TYPE;
        }

        template < typename K >
        K* get()
        {
                return get_impl< K, K >( *this );
        }

        template < typename K >
        const K* get() const
        {
                return get_impl< const K, K >( *this );
        }

        Node* get_node()
        {
                return get_impl< Node, Node >( *this );
        }

        const Node* get_node() const
        {
                return get_impl< const Node, Node >( *this );
        }

        Header* find_header()
        {
                return find_header_impl< Header >( *this );
        }

        const Header* find_header() const
        {
                return find_header_impl< const Header >( *this );
        }

        void match( auto&& tf, auto&& uf )
        {
                match_impl( *this, tf, uf );
        }

        void match( auto&& tf, auto&& uf ) const
        {
                match_impl( *this, tf, uf );
        }

        bool operator==( std::nullptr_t ) const
        {
                return ptr_ == nullptr;
        }

        bool operator==( const list_ptr& other ) const  = default;
        auto operator<=>( const list_ptr& other ) const = default;

private:
        template < typename RetType, typename K >
        static RetType* get_impl( auto& self )
        {
                static_assert( std::same_as< K, Header > || std::same_as< K, Node > );

                if constexpr ( std::same_as< K, Header > ) {
                        if ( self.type() == mark::HEADER_TYPE )
                                return reinterpret_cast< RetType* >( self.pure_ptr() );
                } else {
                        if ( self.type() == mark::NODE_TYPE )
                                return reinterpret_cast< RetType* >( self.pure_ptr() );
                }
                return nullptr;
        }

        static void match_impl( auto& self, auto&& tf, auto&& uf )
        {
                if ( auto* ptr = self.template get< Header >() )
                        uf( *ptr );
                else if ( auto* ptr = self.get_node() )
                        tf( *ptr );
        }

        template < typename RetType >
        static RetType* find_header_impl( auto& self )
        {
                if ( auto* ptr = self.template get< Header >() )
                        return ptr;
                else if ( auto* ptr = self.get_node() )
                        return &Accessor::get( *ptr );
                return nullptr;
        }

        [[nodiscard]] uint8_t ctlbit() const
        {
                return reinterpret_cast< std::uintptr_t >( ptr_ ) & 0x01;
        }

        static constexpr std::uintptr_t mask = ~static_cast< std::uintptr_t >( 0x01 );

        [[nodiscard]] void* pure_ptr() const
        {
                auto raw = reinterpret_cast< std::uintptr_t >( ptr_ );
                raw      = raw & mask;
                return reinterpret_cast< void* >( raw );
        }

        void* ptr_ = nullptr;
};

}  // namespace joque::bits
