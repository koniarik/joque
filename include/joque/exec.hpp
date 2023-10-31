#pragma once

#include "joque/coro.hpp"
#include "joque/records.hpp"
#include "joque/task.hpp"

namespace joque
{

using exec_coro_t = task_coro< exec_record >;

exec_coro_t exec( const task_set& ts, unsigned thread_count = 0, const std::string& filter = "" );

}  // namespace joque
