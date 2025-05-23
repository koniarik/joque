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
#include "joque/junit.hpp"

#include "joque/records.hpp"

#include <chrono>
#include <filesystem>
#include <format>
#include <fstream>
#include <numeric>
#include <ostream>
#include <string>
#include <utility>

namespace joque
{
namespace
{
        using sec_time = std::chrono::duration< double >;

        struct xml_node
        {
                std::ostream& os;
                std::string   name;

                xml_node( std::ostream& os, std::string node_name, auto&&... args )
                  : os( os )
                  , name( std::move( node_name ) )
                {
                        os << "<" << name;
                        ( ( os << " " << args ), ... );
                        os << ">\n";
                }

                ~xml_node()
                {
                        os << "</" << name << ">\n";
                }
        };

        auto attr( auto&& fmt, auto&& arg )
        {
                return std::vformat( std::string{ fmt } + "=\"{}\"", std::make_format_args( arg ) );
        };
}  // namespace

void generate_junit_xml(
    const std::filesystem::path& p,
    const std::string&           ts_name,
    const exec_record&           exec_rec )
{
        std::ofstream of{ p };
        generate_junit_xml( of, ts_name, exec_rec );
}

void generate_junit_xml( std::ostream& os, const std::string& ts_name, const exec_record& exec_rec )
{
        auto t = runtime_sum( exec_rec );

        os << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";

        const xml_node ns{
            os,
            "testsuite",
            attr( "name", ts_name ),
            attr( "tests", exec_rec.runs.size() ),
            attr( "failures", exec_rec.stats.at( run_status::FAIL ) ),
            attr( "time", t ),
            attr( "timestamp", std::format( "{:%FT%TZ}", std::chrono::system_clock::now() ) ) };

        for ( const run_record& rec : exec_rec.runs ) {
                const xml_node tc{
                    os,
                    "testcase",
                    attr( "name", rec.name ),
                    attr(
                        "time",
                        std::chrono::duration_cast< sec_time >( rec.end - rec.start ).count() ) };

                if ( rec.retcode != 0 )
                        os << "<failure message=\"task failed\"/>\n";
        }
}

}  // namespace joque
