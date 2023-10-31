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

        operator std::tuple< int&, std::string&, std::string& >() &&
        {
                return { retcode, sout, serr };
        }
};

template < typename T >
struct job_traits
{
        static bool is_invalidated( const T& )
        {
                return true;
        }
        static run_result run( const task* t, const T& f )
        {
                return f( t );
        }
};

}  // namespace joque
