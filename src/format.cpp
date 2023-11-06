#include "joque/format.hpp"

#include <iostream>

namespace joque
{

void format_nested( std::ostream& os, std::string_view indent, std::string_view msg )
{
        std::string_view::size_type pos  = 0;
        std::string_view::size_type next = 0;
        while ( next != std::string_view::npos ) {
                next = msg.find( '\n', pos );
                os << indent << msg.substr( pos, next - pos ) << '\n';
                pos = next + 1;
        }
}

void format_record( std::ostream& os, const run_record& rec )
{
        os << std::format(
            " {:>5}  {:<60} ",
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

        if ( rec.retcode != 0 ) {
                format_nested( os, "      ", rec.std_out );
                format_nested( os, "      ", rec.std_err );
        }
}

}  // namespace joque
