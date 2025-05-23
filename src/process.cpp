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
#include "joque/process.hpp"

#include "joque/run_result.hpp"
#include "joque/traits.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <regex>
#include <reproc++/drain.hpp>
#include <reproc++/reproc.hpp>
#include <string>
#include <system_error>
#include <tuple>
#include <utility>

namespace joque
{

namespace
{
        std::filesystem::file_time_type path_to_write_time( const std::filesystem::path& p )
        {
                if ( exists( p ) )
                        return last_write_time( p );
                return {};
        }

        std::string format_cmd( const std::vector< std::string >& cmd )
        {
                std::string cmdline;
                std::regex  re{ " " };
                for ( const std::string& arg : cmd )
                        cmdline += std::regex_replace( arg, re, "\\ " ) + " ";
                cmdline += "\n";
                return cmdline;
        }
}  // namespace

process process::add_output( std::filesystem::path p ) &&
{
        output.push_back( std::move( p ) );
        return *this;
}

process process::add_input( std::filesystem::path p ) &&
{
        input.push_back( ( std::move( p ) ) );
        return *this;
}

process process::set_retcode_file( std::filesystem::path p ) &&
{
        retcode_file = std::move( p );
        return *this;
}

inval_result job_traits< process >::is_invalidated( const process& p )
{
        if ( p.retcode_file ) {
                if ( !std::filesystem::exists( *p.retcode_file ) )
                        return { true, "no retcode file" + p.retcode_file->string() };

                int           last_retcode = 0;
                std::ifstream retfile( *p.retcode_file );
                retfile >> last_retcode;
                if ( last_retcode != 0 )
                        return {
                            true,
                            std::format(
                                "last retcode is nonzero: {} in {}",
                                last_retcode,
                                p.retcode_file->string() ) };
        }

        if ( p.output.empty() )
                return { true, "no output" };

        if ( p.input.empty() ) {
                bool out_exists =
                    std::ranges::all_of( p.output, [&]( const std::filesystem::path& p ) {
                            return exists( p );
                    } );

                return { out_exists, "output existance" };
        }

        auto oldest_output_iter =
            std::ranges::min_element( p.output, std::ranges::less{}, path_to_write_time );

        if ( !exists( *oldest_output_iter ) )
                return { true, "missing output: " + oldest_output_iter->string() };

        auto latest_input_iter =
            std::ranges::max_element( p.input, std::ranges::less{}, path_to_write_time );

        auto oldest_output_t = path_to_write_time( *oldest_output_iter );
        auto latest_input_t  = path_to_write_time( *latest_input_iter );

        return {
            .invalidated = oldest_output_t <= latest_input_t,
            .log         = std::format(
                "oldest output {} t: {} \n latest input {} t: {}",
                oldest_output_iter->string(),
                oldest_output_t,
                latest_input_iter->string(),
                latest_input_t ),
        };
}

run_result job_traits< process >::run( const task&, const process& p )
{
        run_result      res;
        reproc::process process;
        reproc::options opts;
        opts.redirect.err.type = reproc::redirect::pipe;

        res.log = "cmd: " + format_cmd( p.cmd );

        std::error_code ec = process.start( p.cmd, opts );

        if ( ec ) {
                if ( ec == std::errc::no_such_file_or_directory )
                        insert_err(
                            res, "Program not found. Make sure it's available from the PATH.\n" );
                else
                        insert_err( res, ec.message() );
                res.retcode = ec.value();
                return res;
        }

        auto f =
            [&]( reproc::stream s, const uint8_t* buffer, std::size_t size ) -> std::error_code {
                insert(
                    res,
                    s == reproc::stream::out ? output_chunk::STANDARD : output_chunk::ERROR,
                    std::string( reinterpret_cast< const char* >( buffer ), size ) );
                return {};
        };
        ec = reproc::drain( process, f, f );
        if ( ec ) {
                insert_err( res, ec.message() );
                res.retcode = ec.value();
                return res;
        }

        std::tie( res.retcode, ec ) = process.wait( reproc::infinite );
        if ( ec ) {
                insert_err( res, ec.message() );
                res.retcode = ec.value();
                return res;
        }

        if ( res.retcode != 0 ) {
                const std::string cmdline = "cmd: " + format_cmd( p.cmd );
                res.output.emplace_front( output_chunk::STANDARD, cmdline );
        }

        if ( p.retcode_file.has_value() ) {
                std::filesystem::remove( *p.retcode_file );
                std::ofstream retof{ *p.retcode_file };
                retof << res.retcode;
        }

        return res;
}

}  // namespace joque
