#include "joque/records.hpp"

#include <algorithm>
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
        auto min = std::ranges::min_element(
            erec.runs, [&]( auto const& lh, auto const& rh ) {
                    return lh.start < rh.start;
            } );
        auto max = std::ranges::max_element(
            erec.runs, [&]( auto const& lh, auto const& rh ) {
                    return lh.end < rh.end;
            } );
        return std::chrono::ceil< std::chrono::seconds >(
            max->end - min->start );
}

}  // namespace joque
