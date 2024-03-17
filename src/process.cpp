#include "joque/process.hpp"

#include "joque/run_result.hpp"
#include "joque/traits.hpp"

#include <algorithm>
#include <bits/ranges_algo.h>
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
#include <string_view>
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

bool job_traits< process >::is_invalidated( const process& p )
{
        if ( p.retcode_file ) {
                if ( !std::filesystem::exists( *p.retcode_file ) )
                        return true;
                int           last_retcode = 0;
                std::ifstream retfile( *p.retcode_file );
                retfile >> last_retcode;
                if ( last_retcode != 0 )
                        return true;
        }

        if ( p.output.empty() )
                return true;

        if ( p.input.empty() ) {
                return std::ranges::all_of( p.output, [&]( const std::filesystem::path& p ) {
                        return exists( p );
                } );
        }

        std::input_iterator auto oldest_output_iter =
            std::ranges::min_element( p.output, std::ranges::less{}, path_to_write_time );

        if ( !exists( *oldest_output_iter ) )
                return true;

        std::input_iterator auto latest_input_iter =
            std::ranges::max_element( p.input, std::ranges::less{}, path_to_write_time );

        auto oldest_output_t = path_to_write_time( *oldest_output_iter );

        auto latest_input_t = path_to_write_time( *latest_input_iter );
        return oldest_output_t <= latest_input_t;
}

run_result job_traits< process >::run( const task&, const process& p )
{
        run_result      res;
        reproc::process process;
        reproc::options opts;
        opts.redirect.err.type = reproc::redirect::pipe;

        std::error_code ec = process.start( p.cmd, opts );

        if ( ec == std::errc::no_such_file_or_directory ) {
                res.output.emplace_back(
                    output_chunk::ERROR,
                    "Program not found. Make sure it's available from the PATH.\n" );
                res.retcode = ec.value();
                return res;
        } else if ( ec ) {
                res.output.emplace_back( output_chunk::ERROR, ec.message() );
                res.retcode = ec.value();
                return res;
        }


        auto f =
            [&]( reproc::stream s, const uint8_t* buffer, std::size_t size ) -> std::error_code {
                record_output(
                    res,
                    s == reproc::stream::out ? output_chunk::STANDARD : output_chunk::ERROR,
                    std::string_view( reinterpret_cast< const char* >( buffer ), size ) );
                return {};
        };
        ec = reproc::drain( process, f, f );
        if ( ec ) {
                record_output( res, output_chunk::ERROR, ec.message() );
                res.retcode = ec.value();
                return res;
        }

        std::tie( res.retcode, ec ) = process.wait( reproc::infinite );
        if ( ec ) {
                record_output( res, output_chunk::ERROR, ec.message() );
                res.retcode = ec.value();
                return res;
        }

        if ( res.retcode != 0 ) {
                std::string cmdline = "cmd: ";
                for ( const std::string& arg : p.cmd )
                        cmdline += std::regex_replace( arg, std::regex{ " " }, "\\ " ) + " ";
                cmdline += "\n";

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
