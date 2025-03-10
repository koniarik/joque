/// MIT License
///
/// Copyright (c) 2025 Jan Veverak Koniarik
///
/// Permission is hereby granted, free of charge, to any person obtaining a copy
/// of this software and associated documentation files (the "Software"), to deal
/// in the Software without restriction, including without limitation the rights
/// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
/// copies of the Software, and to permit persons to whom the Software is
/// furnished to do so, subject to the following conditions:
///
/// The above copyright notice and this permission notice shall be included in all
/// copies or substantial portions of the Software.
///
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
/// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
/// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
/// SOFTWARE.
#pragma once

#include "dag.hpp"
#include "exec_coro.hpp"
#include "exec_visitor.hpp"
#include "print_exec_visitor.hpp"
#include "task.hpp"

#include <string>
#include <thread>

namespace joque
{

/// Runs execution of tasks within one taskset.
///
/// This returns coroutine handling the execution process. Withing it lives a
/// thread pool that pararelly executes all tasks in the task_set. Tasks are
/// limited only to those that pass the filter, or their dependencies.
///
/// In case any task has defined dependencies on resource, only one task at a
/// time would be executed for each resource.
///
/// During the execution, the input task set shall not be modified and any
/// modification might result in undefined behavior. After the execution, the
/// produced records structure is valid only for as long as the input task_set
/// was not modified.
///
/// In case 1...n threads are requested, the tasks are executed inside those
/// worker threads and control thread is not used for execution. In case the
/// thread count is 0, all jobs are executed within the control thread itself.
///
/// \param ts Input task set to process
/// \param thread_count Number of threads to use
/// \param filter String filtering out tasks that either match it, or are
/// dependencies of matched tasks
/// \param vis Visitor called by the execution on various events
///
[[nodiscard]] exec_coro exec(
    const task_set&    ts,
    unsigned           thread_count = std::thread::hardware_concurrency(),
    const std::string& filter       = "",
    exec_visitor&      vis          = PRINT_VISITOR );

/// Overload of `exec` which uses dag as an input instead of task set. It's not
/// recommended to use it, but it exists in case users want to customize the
/// dag.
[[nodiscard]] exec_coro exec(
    dag           g,
    unsigned      thread_count = std::thread::hardware_concurrency(),
    exec_visitor& vis          = PRINT_VISITOR );

}  // namespace joque
