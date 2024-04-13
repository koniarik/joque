#pragma once

#include <stdexcept>
#include <vector>

namespace joque
{
struct task;

struct jexcp : std::runtime_error
{
        using std::runtime_error::runtime_error;
};

struct cycle_excp : jexcp
{
        cycle_excp( std::vector< const task* > cycle )
          : jexcp( "Cycle detected" )
          , cycle( std::move( cycle ) )
        {
        }

        std::vector< const task* > cycle;
};

}  // namespace joque