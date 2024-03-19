#include "joque/records.hpp"

#include <numeric>

namespace joque
{

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
