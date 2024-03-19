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
            attr( "failures", exec_rec.failed_count ),
            attr( "skipped", exec_rec.skipped_count ),
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
                if ( rec.skipped )
                        os << "<failure message=\"task skipped\"/>\n";
        }
}

}  // namespace joque
