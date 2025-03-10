/// MIT License
///
/// Copyright (c) 2025 Jan Veverak Koniarik
///
/// Permission is hereby granted, free of charge, to any person obtaining a copy
/// of this software and associated documentation files (the "Software"), to deal
/// in the Software without restriction, including without limitation the rights
/// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
/// copies of the Software, and to permit persons to whom the Software is
/// furnished to do so, subject to the following conditions:
///
/// The above copyright notice and this permission notice shall be included in all
/// copies or substantial portions of the Software.
///
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
/// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
/// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
/// SOFTWARE.
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
void format_run_end( std::ostream& os, const exec_record& erec, const run_record& rec );

void format_exec_end( std::ostream& os, const exec_record& erec );

void format_status( std::ostream& os, const exec_record& erec, std::string_view name );

}  // namespace joque
