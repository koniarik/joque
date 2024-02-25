#pragma once

namespace joque::bits
{

struct dag_lheader_accessor
{
        static auto& get( auto& n )
        {
                return n.lheader;
        }
};

struct dag_lrheader_accessor
{
        static auto& get( auto& n )
        {
                return n.lrheader;
        }
};

}  // namespace joque::bits