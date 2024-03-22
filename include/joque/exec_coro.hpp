#pragma once

#include "joque/records.hpp"

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
        struct promise_type
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

        /// Constructed by the coroutine mechanism from withing the promise type
        exec_coro( std::coroutine_handle< promise_type > h );

        /// Move swaps internal pointers
        exec_coro( exec_coro&& other ) noexcept;
        exec_coro& operator=( exec_coro&& other ) noexcept;

        /// Execution is done after coroutine finishes it's run
        [[nodiscard]] bool done() const;

        /// Returns execution record with data on successfull run. Returns empty otherwise.
        /// If exception was raised during the run, result will rethrow it.
        [[nodiscard]] std::optional< exec_record > result();

        /// Runs one iteration of the execution, either new task is executed or nothing happens due
        /// to all threads/resources being utilized. Does nothing after the execution finished.
        void tick();

        /// Blocks until coroutine finishes it's run, returns pointer to exec_record with exact same
        /// behavior as `result`.
        ///
        /// \param period Time between attempts to resume the coroutine
        std::optional< exec_record >
        run( std::chrono::milliseconds period = std::chrono::milliseconds{ 5 } );

        /// Destroys the coroutine if any.
        ~exec_coro();

private:
        std::coroutine_handle< promise_type > h_;
};

}  // namespace joque
