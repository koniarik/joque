#pragma once

#include "joque/out.hpp"
#include "joque/traits.hpp"

#include <filesystem>

namespace joque
{

struct process
{
        std::vector< std::string >           cmd;
        std::vector< std::filesystem::path > input;
        std::vector< std::filesystem::path > output;

        template < typename... Args >
        static process derive( Args&&... args )
        {
                process res;

                auto f = [&]< typename T >( T&& thing ) {
                        if constexpr ( std::derived_from< std::decay_t< T >, out_tag > ) {
                                res.cmd.push_back( thing.p );
                                res.output.push_back( thing.p );
                        } else if constexpr ( std::same_as<
                                                  std::decay_t< T >,
                                                  std::filesystem::path > ) {
                                res.cmd.push_back( thing );
                                res.input.push_back( thing );
                        } else {
                                res.cmd.push_back( thing );
                        }
                };
                ( f( args ), ... );
                return res;
        }

        process add_output( std::filesystem::path p ) &&
        {
                output.push_back( std::move( p ) );
                return *this;
        }
};

template <>
struct job_traits< process >
{
        static bool       is_invalidated( const process& p );
        static run_result run( const task*, const process& p );
};

}  // namespace joque
