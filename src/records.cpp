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

}  // namespace joque
