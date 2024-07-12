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
        static constexpr std::string_view GRAY  = "\033[38;5;239m";
        static constexpr std::string_view OK    = "\033[38;5;118m";
        static constexpr std::string_view SKIP  = "\033[38;5;226m";
        static constexpr std::string_view DEPF  = "\033[38;5;202m";
        static constexpr std::string_view FAIL  = "\033[38;5;196m";
        static constexpr std::string_view END   = "\033[38;5;45m";
        static constexpr std::string_view NONE  = "";
        static constexpr std::string_view RESET = "\033[0m";
        // TODO: hardcoded yikes
        static constexpr std::string_view GRAY_DELIM = "\033[38;5;239m/\033[0m";

        static const std::map< run_status, std::string_view > STAT_TO_COLOR{
            { run_status::OK, OK },
            { run_status::SKIP, SKIP },
            { run_status::DEPF, DEPF },
            { run_status::FAIL, FAIL },
        };

        std::string fmt_time()
        {
                auto now = std::chrono::system_clock::now();
                auto s =
                    std::chrono::time_point_cast< std::chrono::seconds >( now );
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

        std::string fmt_text( std::string text )
        {
                std::string str = std::format( "{:<60}", text );

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
                            count > 0 ? STAT_TO_COLOR.at( key ) : GRAY,
                            count );
                        text += RESET;
                }
                return text;
        }

}  // namespace

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

void format_record(
    std::ostream&      os,
    const exec_record& erec,
    const run_record&  rec )
{
        os << fmt_time();
        os << "  ";
        os << fmt_counter( erec.runs.size() + 1, erec.total_count ) << "/"
           << GRAY << fmt_counter( erec.total_count, erec.total_count )
           << RESET;
        os << "  ";
        os << fmt_status( to_sv( rec.status ), STAT_TO_COLOR.at( rec.status ) );
        os << "  ";
        os << fmt_text( rec.name );
        os << "  ";
        if ( rec.status == run_status::OK || rec.status == run_status::FAIL )
                os << fmt_dur(
                    std::chrono::duration_cast< std::chrono::milliseconds >(
                        rec.end - rec.start ) );

        os << "\n";
}

void format_end( std::ostream& os, const exec_record& erec )
{
        using namespace std::string_literals;

        auto        max_c    = erec.total_count;
        auto        count_w  = counter_width( max_c );
        std::string count_ws = std::to_string( count_w * 2 + 1 );


        os << fmt_time();
        os << "  ";
        os << std::vformat(
            "{:>" + count_ws + "}", std::make_format_args( erec.total_count ) );
        os << "  ";
        os << fmt_status( "END", END );
        os << "  ";
        os << end_text( erec ) << " ";
        os << "  ";
        os << fmt_dur( runtime_sum( erec ) );
        os << std::endl;
}

}  // namespace joque
