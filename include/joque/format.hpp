#pragma once

#include "records.hpp"
#include "run_result.hpp"

#include <list>
#include <ostream>
#include <string_view>

namespace joque
{

/// Assumes that `msg` is multiline message, streams each line into `os`, and
/// prefixes each line with `indent` string.
void format_nested(
    std::ostream&                    os,
    std::string_view                 indent,
    const std::list< output_chunk >& output );

/// Formats information about record `rec` and streams it into `os`.
void format_run_end(
    std::ostream&      os,
    const exec_record& erec,
    const run_record&  rec );

void format_exec_end( std::ostream& os, const exec_record& erec );

void format_status(
    std::ostream&      os,
    const exec_record& erec,
    std::string_view   name );

}  // namespace joque
