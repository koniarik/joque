#include "joque/format.hpp"

#include "joque/records.hpp"
#include "joque/run_result.hpp"

#include <chrono>
#include <cmath>
#include <format>
#include <iostream>
#include <list>
#include <regex>
#include <string_view>

namespace joque
{
namespace
{
        static constexpr std::string_view START = "\033[38;5;33m";
        static constexpr std::string_view OK    = "\033[38;5;118m";
        static constexpr std::string_view FAIL  = "\033[38;5;196m";
        static constexpr std::string_view DEPF  = "\033[38;5;202m";
        static constexpr std::string_view SKIP  = "\033[38;5;226m";
        static constexpr std::string_view GRAY  = "\033[38;5;239m";
        static constexpr std::string_view END   = "\033[38;5;45m";
        static constexpr std::string_view NONE  = "";
        static constexpr std::string_view RESET = "\033[0m";
        // TODO: hardcoded yikes
        static constexpr std::string_view GRAY_DELIM = "\033[38;5;239m/\033[0m";

        static const std::map< std::string_view, std::string_view >
            STAT_TO_COLOR{
                { "OK", OK },
                { "SKIP", SKIP },
                { "DEPF", DEPF },
                { "FAIL", FAIL },
                { "STRT", START } };

        std::string fmt_time( const std::chrono::sys_seconds& s )
        {
                return std::format(
                    "{}{:%H}{:%M}{:%S}{}", GRAY, s, s, s, RESET );
        }

        std::size_t counter_width( std::size_t max_count )
        {
                return static_cast< std::size_t >(
                    std::log10( static_cast< double >( max_count ) ) + 1 );
        }

        std::string fmt_counter( std::size_t count, std::size_t max_count )
        {
                auto        count_w  = counter_width( max_count );
                std::string count_ws = std::to_string( count_w );

                return std::vformat(
                    "{:0>" + count_ws + "}",
                    std::make_format_args( count, max_count ) );
        }

        std::string fmt_status( std::string_view stat, std::string_view color )
        {
                return std::format( "{}{:>4}{}", color, stat, RESET );
        }

        std::string fmt_text( std::string_view text )
        {
                std::string str = std::format( "{:<60}", text.substr( 0, 60 ) );

                std::size_t index = 0;
                while ( true ) {
                        index = str.find( '/', index );
                        if ( index == std::string::npos )
                                break;
                        str.replace( index, 1, GRAY_DELIM );
                        index += GRAY_DELIM.size();
                };
                return str;
        }

        std::string fmt_dur( auto t )
        {
                return std::format( "  {}{:>8}{}", GRAY, t, RESET );
        }

        std::string end_text( const exec_record& erec )
        {

                std::string text;
                text += std::format(
                    "run: {:<4}",
                    erec.stats.at( run_status::OK ) +
                        erec.stats.at( run_status::FAIL ) );
                for ( auto&& [key, count] : erec.stats ) {
                        text += "  ";
                        text += std::format(
                            "{}{}: {}{:<5}",
                            count > 0 ? NONE : GRAY,
                            to_sv( key ),
                            count > 0 ? STAT_TO_COLOR.at( to_sv( key ) ) : GRAY,
                            count );
                        text += RESET;
                }
                return text;
        }

        void format_line(
            std::ostream&                              os,
            std::chrono::sys_seconds                   t,
            std::size_t                                count,
            std::optional< std::size_t >               total_count,
            std::string_view                           status,
            std::string_view                           text,
            std::optional< std::chrono::milliseconds > dur )
        {
                os << fmt_time( t );
                os << "  ";
                if ( total_count ) {
                        os << fmt_counter( count, *total_count ) << "/" << GRAY
                           << fmt_counter( *total_count, *total_count )
                           << RESET;
                } else {
                        auto count_w = counter_width( count );
                        for ( std::size_t i = 0; i < count_w + 1; i++ )
                                os << " ";
                        os << fmt_counter( count, count );
                }
                os << "  ";
                auto iter = STAT_TO_COLOR.find( status );
                if ( iter == STAT_TO_COLOR.end() )
                        os << fmt_status( status, NONE );
                else
                        os << fmt_status( status, iter->second );
                os << "  ";
                os << text;
                os << "  ";
                if ( dur )
                        os << fmt_dur( *dur );
        }


}  // namespace

void format_run_start(
    std::ostream&      os,
    const exec_record& erec,
    std::string_view   name )
{
        auto now = std::chrono::system_clock::now();
        auto s   = std::chrono::time_point_cast< std::chrono::seconds >( now );
        format_line(
            os,
            s,
            erec.runs.size() + 1,
            erec.total_count,
            "STRT",
            fmt_text( name ),
            std::nullopt );
}

void format_nested(
    std::ostream&                    os,
    std::string_view                 indent,
    const std::list< output_chunk >& output )
{
        std::string_view prefix = indent;
        char             lc     = '\n';
        for ( const output_chunk& ch : output )
                for ( const char c : ch.data ) {
                        lc = c;
                        os << prefix << c;
                        if ( c == '\n' )
                                prefix = indent;
                        else
                                prefix = "";
                }
        if ( lc != '\n' )
                os << '\n';
}

void format_run_end(
    std::ostream&      os,
    const exec_record& erec,
    const run_record&  rec )
{
        auto now = std::chrono::system_clock::now();
        auto s   = std::chrono::time_point_cast< std::chrono::seconds >( now );

        std::optional< std::chrono::milliseconds > dur;
        if ( rec.status == run_status::OK || rec.status == run_status::FAIL )
                dur = std::chrono::duration_cast< std::chrono::milliseconds >(
                    rec.end - rec.start );

        format_line(
            os,
            s,
            erec.runs.size() + 1,
            erec.total_count,
            to_sv( rec.status ),
            fmt_text( rec.name ),
            dur );
}

void format_exec_end( std::ostream& os, const exec_record& erec )
{

        auto now = std::chrono::system_clock::now();
        auto s   = std::chrono::time_point_cast< std::chrono::seconds >( now );

        format_line(
            os,
            s,
            erec.total_count,
            std::nullopt,
            END,
            end_text( erec ),
            runtime_sum( erec ) );
}

}  // namespace joque
