#include "joque/json.hpp"

namespace joque
{

void to_json( nlohmann::json& j, const output_chunk& rec )
{
        map( rec, [&]( const std::string_view key, auto& val ) {
                j[key] = val;
        } );
}

void to_json( nlohmann::json& j, const run_record& rec )
{
        map( rec, [&]< typename T >( const std::string_view key, T& val ) {
                if constexpr ( std::same_as< T, const tp > )
                        j[key] = std::format( "{}", val );
                else if constexpr ( !std::same_as<
                                        T,
                                        const std::reference_wrapper<
                                            const task > > )
                        j[key] = val;
        } );
}

void to_json( nlohmann::json& j, const exec_record& rec )
{
        map( rec, [&]( const std::string_view key, auto& val ) {
                j[key] = val;
        } );
}

}  // namespace joque