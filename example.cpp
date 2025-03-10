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
#include <joque/exec.hpp>

using namespace std::chrono_literals;

joque::task_set gen_tasks( std::size_t n )
{
        joque::task_set res;
        for ( std::size_t i = 0; i < n; i++ ) {
                res.tasks["task_" + std::to_string( i )] = joque::task{
                    .job = []( const joque::task& ) -> joque::run_result {
                            std::this_thread::sleep_for( 200ms );

                            return { 0 };
                    },
                };
        }
        return res;
}

int main( int argc, char* argv[] )
{
        if ( argc != 2 )
                return 1;
        joque::task_set ts;
        int             n = std::stoi( argv[1] );
        ts.sets["a"]      = gen_tasks( n );
        ts.sets["b"]      = gen_tasks( n );
        joque::exec( ts ).run();
}
