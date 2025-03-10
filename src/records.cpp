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
        if ( erec.runs.empty() )
                return std::chrono::seconds{ 0 };
        auto min = std::ranges::min_element( erec.runs, [&]( auto const& lh, auto const& rh ) {
                return lh.start < rh.start;
        } );
        auto max = std::ranges::max_element( erec.runs, [&]( auto const& lh, auto const& rh ) {
                return lh.end < rh.end;
        } );
        return std::chrono::ceil< std::chrono::seconds >( max->end - min->start );
}

}  // namespace joque
