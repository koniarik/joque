#pragma once

#include "joque/task.hpp"

#include <chrono>

namespace joque
{

using tp = std::chrono::time_point< std::chrono::system_clock >;

struct run_record
{
        const task* t;
        std::string name;
        bool        skipped = false;
        int         retcode = 0;
        tp          start   = std::chrono::system_clock::now();
        tp          end     = std::chrono::system_clock::now();
        std::string std_out;
        std::string std_err;
};

struct exec_record
{
        std::size_t               skipped_count = 0;
        std::size_t               failed_count  = 0;
        std::vector< run_record > runs;
};

}  // namespace joque
