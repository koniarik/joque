#pragma once

#include <filesystem>

namespace joque
{

/// Baseclass for tagging system
struct out_tag
{
};

/// Tags a filesystem path as an output filename.
struct out : out_tag
{
        const std::filesystem::path& p;

        out( const std::filesystem::path& p )
          : p( p )
        {
        }
};

}  // namespace joque
