#pragma once

#include <list>
#include <string>
#include <string_view>
#include <tuple>

namespace joque
{


struct output_chunk
{
        enum type_e
        {
                ERROR,
                STANDARD
        };
        type_e      type;
        std::string data;
};

void map( std::convertible_to< output_chunk > auto& rec, auto&& f )
{
        f( "type", rec.type );
        f( "data", rec.data );
}

/// Result of single traits run call. Information is stored in run record.
struct run_result
{
        /// Return code of the run, 0 implies success.
        int retcode;

        std::list< output_chunk > output;

        std::list< std::string > log;
};

void insert( auto& res, output_chunk::type_e type, std::string data )
{
        res.output.emplace_back( type, std::move( data ) );
}

void insert_std( auto& res, std::string data )
{
        insert( res, output_chunk::STANDARD, std::move( data ) );
}

void insert_err( auto& res, std::string data )
{
        insert( res, output_chunk::ERROR, std::move( data ) );
}

void insert_log( auto& res, std::string data )
{
        res.log.emplace_back( std::move( data ) );
}

}  // namespace joque
