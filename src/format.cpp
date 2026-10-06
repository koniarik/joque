/// MIT License
///
/// Copyright (c) 2025-2026 Jan Veverak Koniarik
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
#include "joque/format.hpp"

#include "joque/records.hpp"
#include "joque/run_result.hpp"

#include <chrono>
#include <cmath>
#include <concepts>
#include <cstddef>
#include <format>
#include <iostream>
#include <list>
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <variant>

namespace joque
{
namespace
{
        constexpr std::string_view start = "\033[38;5;33m";
        constexpr std::string_view ok    = "\033[38;5;118m";
        constexpr std::string_view fail  = "\033[38;5;196m";
        constexpr std::string_view depf  = "\033[38;5;202m";
        constexpr std::string_view skip  = "\033[38;5;226m";
        constexpr std::string_view gray  = "\033[38;5;239m";
        constexpr std::string_view end   = "\033[38;5;45m";
        constexpr std::string_view none;
        constexpr std::string_view reset = "\033[0m";
        // TODO: hardcoded yikes
        constexpr std::string_view gray_delim = "\033[38;5;239m/\033[0m";

        const std::map< std::string_view, std::string_view > stat_to_color{
            { "OK", ok },
            { "SKIP", skip },
            { "DEPF", depf },
            { "FAIL", fail },
            { "STRT", start } };

        std::string fmt_time( const std::chrono::sys_seconds& s )
        {
                return std::format( "{}{:%H}{:%M}{:%S}{}", gray, s, s, s, reset );
        }

        std::size_t counter_width( std::size_t max_count )
        {
                return static_cast< std::size_t >(
                    std::log10( static_cast< double >( max_count ) ) + 1 );
        }

        std::string fmt_counter( std::size_t count, std::size_t max_count )
        {
                auto              count_w  = counter_width( max_count );
                std::string const count_ws = std::to_string( count_w );

                return std::vformat(
                    "{:0>" + count_ws + "}", std::make_format_args( count, max_count ) );
        }

        std::string fmt_status( std::string_view stat, std::string_view color )
        {
                return std::format( "{}{:>4}{}", color, stat, reset );
        }

        std::string fmt_text( std::string_view text )
        {
                std::string str = std::format( "{:<60}", text.substr( 0, 60 ) );

                std::size_t index = 0;
                while ( true ) {
                        index = str.find( '/', index );
                        if ( index == std::string::npos )
                                break;
                        str.replace( index, 1, gray_delim );
                        index += gray_delim.size();
                };
                return str;
        }

        std::string fmt_dur( auto t )
        {
                return std::format( "  {}{:>8}{}", gray, t, reset );
        }

        std::string end_text( const exec_record& erec )
        {

                std::string text;
                text += std::format(
                    "run: {:<9}",
                    erec.stats.at( run_status::OK ) + erec.stats.at( run_status::FAIL ) );
                for ( auto&& [key, count] : erec.stats ) {
                        text += "  ";
                        text += std::format(
                            "{}{}: {}{:<5}",
                            count > 0 ? none : gray,
                            to_sv( key ),
                            count > 0 ? stat_to_color.at( to_sv( key ) ) : gray,
                            count );
                        text += reset;
                }
                return text;
        }

        using time_var =
            std::variant< std::monostate, std::chrono::milliseconds, std::chrono::seconds >;

        void format_line(
            std::ostream&                os,
            std::chrono::sys_seconds     t,
            std::size_t                  count,
            std::optional< std::size_t > total_count,
            std::string_view             status,
            std::string_view             text,
            time_var                     dur )
        {
                os << fmt_time( t );
                os << "  ";
                if ( total_count ) {
                        os << fmt_counter( count, *total_count ) << "/" << gray
                           << fmt_counter( *total_count, *total_count ) << reset;
                } else {
                        auto count_w = counter_width( count );
                        for ( std::size_t i = 0; i < count_w + 1; i++ )
                                os << " ";
                        os << fmt_counter( count, count );
                }
                os << "  ";
                auto iter = stat_to_color.find( status );
                if ( iter == stat_to_color.end() )
                        os << fmt_status( status, none );
                else
                        os << fmt_status( status, iter->second );
                os << "  ";
                os << text;
                os << "  ";
                std::visit(
                    [&]< typename T >( T& item ) {
                            if constexpr ( !std::same_as< T, std::monostate > )
                                    os << fmt_dur( item );
                    },
                    dur );
        }

}  // namespace

void format_status( std::ostream& os, const exec_record& erec, std::string_view name )
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
            std::monostate{} );
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

void format_run_end( std::ostream& os, const exec_record& erec, const run_record& rec )
{
        auto now = std::chrono::system_clock::now();
        auto s   = std::chrono::time_point_cast< std::chrono::seconds >( now );

        time_var dur;
        if ( rec.status == run_status::OK || rec.status == run_status::FAIL )
                dur =
                    std::chrono::duration_cast< std::chrono::milliseconds >( rec.end - rec.start );

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
            os, s, erec.total_count, std::nullopt, end, end_text( erec ), runtime_sum( erec ) );
}

}  // namespace joque
