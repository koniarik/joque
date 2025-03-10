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

#include "records.hpp"

#include <chrono>
#include <coroutine>
#include <exception>
#include <optional>

namespace joque
{

/// Coroutine representing one execution of entire task set.
class exec_coro
{
public:
        struct promise_type;

        /// Constructed by the coroutine mechanism from withing the promise type
        exec_coro( std::coroutine_handle< promise_type > h );

        /// Move swaps internal pointers
        exec_coro( exec_coro&& other ) noexcept;
        exec_coro& operator=( exec_coro&& other ) noexcept;

        /// Execution is done after coroutine finishes it's run
        [[nodiscard]] bool done() const;

        /// Returns execution record with data on successfull run. Returns empty
        /// otherwise. If exception was raised during the run, result will
        /// rethrow it.
        [[nodiscard]] std::optional< exec_record > result();

        /// Runs one iteration of the execution, either new task is executed or
        /// nothing happens due to all threads/resources being utilized. Does
        /// nothing after the execution finished.
        void tick();

        /// Blocks until coroutine finishes it's run, returns pointer to
        /// exec_record with exact same behavior as `result`.
        ///
        /// \param period Time between attempts to resume the coroutine
        std::optional< exec_record >
        run( std::chrono::milliseconds period = std::chrono::milliseconds{ 5 } );

        /// Destroys the coroutine if any.
        ~exec_coro();

private:
        std::coroutine_handle< promise_type > h_;
};

struct exec_coro::promise_type
{
        [[nodiscard]] exec_coro get_return_object();

        [[nodiscard]] std::suspend_always initial_suspend() const;

        [[nodiscard]] std::suspend_always final_suspend() const noexcept;

        void unhandled_exception();

        void return_value( exec_record rec );

        /// Value returned at the end of execution
        std::optional< exec_record > value;

        /// Potential exception raised from within the coroutine
        std::exception_ptr excep;
};

}  // namespace joque
