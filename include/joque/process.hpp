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

#include "out.hpp"
#include "run_result.hpp"
#include "traits.hpp"

#include <concepts>
#include <filesystem>
#include <optional>
#include <string>
#include <type_traits>
#include <vector>

namespace joque
{

/// Job-friendly structure for definition of subprocess to be executed. This
/// closely mirrors native `make` behavior.
///
/// Subprocess is defined by a `cmd` as a vector of strings. Contains list of
/// input and output files, if any of the input files is more recent than any
/// of the output files, process is marked as invalidated.
struct process
{
        /// cmd for the process execution
        std::vector< std::string > cmd;
        /// input files that shall be monitored for changes
        std::vector< std::filesystem::path > input;
        /// output files that shall be monitored for timestamps
        std::vector< std::filesystem::path > output;

        std::optional< std::filesystem::path > retcode_file;  // TODO: write
                                                              // tests for this

        /// Takes input command for process as any arguments and builds process
        /// instance out of those.
        ///
        /// In case std::filesystem::path is present, it is considered an input
        /// file and added to input attribute.
        ///
        /// In case std::filesystem::path wrapped in out is present, it is
        /// unwrapped and considered an output fil and added to the output
        /// attribute.
        template < typename... Args >
        [[nodiscard]] static process derive( Args&&... args );

        /// Adds an output path to the output files. API is designed for tail
        /// chaining.
        process add_output( std::filesystem::path p ) &&;

        process add_input( std::filesystem::path p ) &&;

        process set_retcode_file( std::filesystem::path p ) &&;
};

/// Traits overload so that process can be used in combination with the library.
///
/// Uses reproc++ internaly for execution of the subprocess.
template <>
struct job_traits< process >
{
        /// Proces becames invalidated in case one of it's input files is more
        /// recent than any of output files.
        [[nodiscard]] static inval_result is_invalidated( const process& p );

        /// Executes the process, captures output of both stdout and stderr.
        /// Returns failure in case return code of subprocess is not 0.
        [[nodiscard]] static run_result run( const task&, const process& p );
};

template < typename... Args >
process process::derive( Args&&... args )
{
        process res;

        auto f = [&]< typename T >( T&& thing ) {
                if constexpr ( std::derived_from< std::decay_t< T >, out_tag > ) {
                        res.cmd.push_back( thing.p );
                        res.output.push_back( thing.p );
                } else if constexpr ( std::same_as< std::decay_t< T >, std::filesystem::path > ) {
                        res.cmd.push_back( thing );
                        res.input.push_back( thing );
                } else {
                        res.cmd.push_back( thing );
                }
        };
        ( f( args ), ... );
        return res;
}

}  // namespace joque
