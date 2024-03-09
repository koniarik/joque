#include "joque/format.hpp"

#include "joque/records.hpp"
#include "joque/run_result.hpp"

#include <chrono>
#include <format>
#include <iostream>
#include <list>
#include <string_view>

namespace joque
{

void format_nested(
    std::ostream&                    os,
    std::string_view                 indent,
    const std::list< output_chunk >& output )
{
        for ( const output_chunk& ch : output )
                for ( const char c : ch.data )
                        if ( c != '\n' )
                                os << c;
                        else
                                os << indent << '\n';
}

void format_record( std::ostream& os, const run_record& rec )
{
        os << std::format(
            " {:>7}  {:<60} ",
            rec.skipped      ? "SKIPPED" :
            rec.retcode == 0 ? "OK" :
                               "FAIL",
            rec.name );

        if ( !rec.skipped ) {
                os << std::format(
                    "({})",
                    std::chrono::duration_cast< std::chrono::milliseconds >(
                        rec.end - rec.start ) );
        }

        os << "\n";

        if ( rec.retcode != 0 )
                format_nested( os, "      ", rec.output );
}

}  // namespace joque
