#pragma once

#include "joque/exec_coro.hpp"

namespace joque
{

[[nodiscard]] exec_coro
exec( const task_set& ts, unsigned thread_count = 0, const std::string& filter = "" );

class dag;
[[nodiscard]] exec_coro exec( dag g, unsigned thread_count = 0, const std::string& filter = "" );

}  // namespace joque
