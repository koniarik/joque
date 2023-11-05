#pragma once

#include <string>
#include <tuple>
#include <vector>

namespace joque
{

struct task;

struct run_result
{
        int         retcode;
        std::string sout;
        std::string serr;

        [[nodiscard]] operator std::tuple< int&, std::string&, std::string& >() &&
        {
                return { retcode, sout, serr };
        }
};

template < typename T >
struct job_traits
{
        [[nodiscard]] static bool is_invalidated( const T& )
        {
                return true;
        }
        [[nodiscard]] static run_result run( const task* t, const T& f )
        {
                return f( t );
        }
};

}  // namespace joque
