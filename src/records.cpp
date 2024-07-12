#include "joque/records.hpp"

#include <chrono>
#include <numeric>
#include <string_view>

namespace joque
{

std::string_view to_sv( const run_status& s )
{
        switch ( s ) {
        case joque::run_status::OK:
                return "OK";
        case joque::run_status::SKIP:
                return "SKIP";
        case joque::run_status::DEPF:
                return "DEPF";
        case joque::run_status::FAIL:
                return "FAIL";
        }
        return "";
}

std::chrono::seconds runtime_sum( const exec_record& erec )
{
        return std::chrono::ceil< std::chrono::seconds >( std::accumulate(
            erec.runs.begin(),
            erec.runs.end(),
            std::chrono::system_clock::duration{ 0u },
            [&]( auto sum, const run_record& rec ) {
                    return sum + ( rec.end - rec.start );
            } ) );
}

#ifdef NLOHMANN_JSON_NAMESPACE_VERSION

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

#endif

}  // namespace joque
