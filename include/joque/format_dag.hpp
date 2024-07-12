#pragma once

#include "dag.hpp"

#include <functional>
#include <ostream>
#include <string_view>

namespace joque
{

// Formats the given dag `d` into human readable format. Output is done by
// calling `f` for each line of the output.
void format_dag(
    const dag&                                       d,
    const std::function< void( std::string_view ) >& f );

// Prints the formatted dag into ostream.
void print_dag( std::ostream& os, const dag& d );

}  // namespace joque
