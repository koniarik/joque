#include "joque/process.hpp"

#include <algorithm>
#include <format>
#include <iostream>
#include <reproc++/drain.hpp>
#include <reproc++/reproc.hpp>

namespace joque
{

namespace
{
        std::filesystem::file_time_type path_to_write_time( const std::filesystem::path& p )
        {
                if ( exists( p ) ) {
                        return last_write_time( p );
                }
                return {};
        }
}  // namespace

process process::add_output( std::filesystem::path p ) &&
{
        output.push_back( std::move( p ) );
        return *this;
}

bool job_traits< process >::is_invalidated( const process& p )
{
        std::input_iterator auto oldest_output_t =
            std::ranges::min_element( p.output, std::ranges::less{}, path_to_write_time );

        std::input_iterator auto latest_file_t =
            std::ranges::max_element( p.input, std::ranges::less{}, path_to_write_time );

        return *oldest_output_t <= *latest_file_t;
}

run_result job_traits< process >::run( const task*, const process& p )
{
        run_result      res;
        reproc::process process;
        reproc::options opts;
        opts.redirect.err.type = reproc::redirect::pipe;

        std::error_code ec = process.start( p.cmd, opts );

        if ( ec == std::errc::no_such_file_or_directory ) {
                res.std_err = "Program not found. Make sure it's available from the PATH.\n";
                res.retcode = ec.value();
                return res;
        } else if ( ec ) {
                res.std_err = ec.message();
                res.retcode = ec.value();
                return res;
        }

        reproc::sink::string sink_out( res.std_out );
        reproc::sink::string sink_err( res.std_err );
        ec = reproc::drain( process, sink_out, sink_err );
        if ( ec ) {
                res.std_err = ec.message();
                res.retcode = ec.value();
                return res;
        }

        std::tie( res.retcode, ec ) = process.wait( reproc::infinite );
        if ( ec ) {
                res.std_err = ec.message();
                res.retcode = ec.value();
                return res;
        }

        if ( res.retcode != 0 ) {
                std::string newout = "cmd: ";
                for ( const std::string& arg : p.cmd ) {
                        newout += arg + " ";
                }
                newout += "\n" + res.std_out;
                res.std_out = newout;
        }

        return res;
}

}  // namespace joque
