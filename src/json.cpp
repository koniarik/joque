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
#include "joque/json.hpp"

namespace joque
{

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
                else if constexpr ( !std::same_as< T, const std::reference_wrapper< const task > > )
                        j[key] = val;
        } );
}

void to_json( nlohmann::json& j, const exec_record& rec )
{
        map( rec, [&]( const std::string_view key, auto& val ) {
                j[key] = val;
        } );
}

}  // namespace joque
