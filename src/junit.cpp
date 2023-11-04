#include "joque/junit.hpp"

#include <chrono>
#include <format>
#include <numeric>

namespace joque
{

using sec_time = std::chrono::duration< double >;

struct xml_node
{
        std::ostream& os;
        std::string   name;

        xml_node( std::ostream& os, std::string name, auto&&... args )
          : os( os )
          , name( name )
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

void render_junit_xml( std::ostream& os, const std::string& ts_name, const exec_record& exec_rec )
{
        auto t = std::chrono::duration_cast< sec_time >(
                     std::accumulate(
                         exec_rec.runs.begin(),
                         exec_rec.runs.end(),
                         std::chrono::system_clock::duration{ 0u },
                         [&]( auto sum, const run_record& rec ) {
                                 return sum + ( rec.end - rec.start );
                         } ) )
                     .count();

        os << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";

        xml_node ns{
            os,
            "testsuite",
            attr( "name", ts_name ),
            attr( "tests", exec_rec.runs.size() ),
            attr( "failures", exec_rec.failed_count ),
            attr( "skipped", exec_rec.skipped_count ),
            attr( "time", t ),
            attr( "timestamp", std::format( "{:%FT%TZ}", std::chrono::system_clock::now() ) ) };

        //        data["commit"]   = git::CommitSHA1();

        for ( const run_record& rec : exec_rec.runs ) {
                xml_node tc{
                    os,
                    "testcase",
                    attr( "name", rec.name ),
                    attr(
                        "time",
                        std::chrono::duration_cast< sec_time >( rec.end - rec.start ).count() ) };

                if ( rec.retcode != 0 ) {
                        os << "<failure message=\"task failed\"/>\n";
                }
                if ( rec.skipped ) {
                        os << "<failure message=\"task skipped\"/>\n";
                }
                if ( rec.std_out.size() != 0 ) {
                        xml_node sout{ os, "std_out" };
                        os << std::accumulate(
                            rec.std_out.begin(), rec.std_out.end(), std::string{ "" } );
                }
                if ( rec.std_err.size() != 0 ) {
                        xml_node serr{ os, "std_err" };
                        os << std::accumulate(
                            rec.std_err.begin(), rec.std_err.end(), std::string{ "" } );
                }
        }
}

}  // namespace joque
