#pragma once

#include "joque/task.hpp"

namespace joque
{

struct node
{
        std::string          name;
        const task*          t;
        std::vector< node* > depends_on{};
        std::vector< node* > after{};

        bool failed      = false;
        bool blocked     = false;
        bool done        = false;
        bool invalidated = false;

        bool is_leaf() const
        {
                return depends_on.empty();
        }
};

struct dag
{
        std::list< node > nodes;
};

dag generate_dag( const task_set& ts, const std::string& filter );

}  // namespace joque
