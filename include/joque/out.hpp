#pragma once

#include <filesystem>

namespace joque
{

struct out_tag
{
};

struct out : out_tag
{
        const std::filesystem::path& p;

        out( const std::filesystem::path& p )
          : p( p )
        {
        }
};

}  // namespace joque
