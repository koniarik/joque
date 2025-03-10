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

#include "run_result.hpp"

namespace joque
{

struct task;

/// Default job traits for all types. Assumes that the type is callable with
/// signature `run_result(const task*)`.
template < typename T >
struct job_traits
{
        [[nodiscard]] static inval_result is_invalidated( const T& )
        {
                return { .invalidated = true, .log = { "always invalidated" } };
        }

        [[nodiscard]] static run_result run( const task& t, T& f )
        {
                try {
                        return f( t );
                }
                catch ( std::exception& e ) {
                        run_result res;
                        insert_err( res, "Uncaught exception:" );
                        insert_err( res, e.what() );
                        return res;
                }
        }
};

}  // namespace joque
