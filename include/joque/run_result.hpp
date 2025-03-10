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

#include <concepts>
#include <cstdint>
#include <list>
#include <string>
#include <string_view>
#include <tuple>

namespace joque
{

struct output_chunk
{
        enum type_e : uint8_t
        {
                ERROR,
                STANDARD
        };

        type_e      type;
        std::string data;
};

void map( std::convertible_to< output_chunk > auto& rec, auto&& f )
{
        f( "type", rec.type );
        f( "data", rec.data );
}

/// TODO: hide this
/// Result of single traits run call. Information is stored in run record.
struct run_result
{
        /// Return code of the run, 0 implies success.
        int retcode;

        std::list< output_chunk > output;

        std::string log;
};

struct inval_result
{
        bool invalidated;

        std::string log;
};

void insert( auto& res, output_chunk::type_e type, std::string data )
{
        res.output.emplace_back( type, std::move( data ) );
}

void insert_std( auto& res, std::string data )
{
        insert( res, output_chunk::STANDARD, std::move( data ) );
}

void insert_err( auto& res, std::string data )
{
        insert( res, output_chunk::ERROR, std::move( data ) );
}

}  // namespace joque
