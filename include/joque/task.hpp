#pragma once

#include "joque/job.hpp"

#include <functional>
#include <map>

namespace joque
{

/// Abstraction to model resource used by tasks. Out of all tasks that reference single resource,
/// only one may be executed at any point in time.
struct resource
{
        std::string name;
};

/// Single task that should be executed by the system.
struct task
{
        /// Job being executed for the task
        job_ptr job;

        /// Dependencies of the task - all of these should be executed before this task. In case any
        /// of these is invalidated, this task is also invalidated.
        std::vector< std::reference_wrapper< const task > > depends_on = {};
        /// Tasks that should be executed before this task. (`depends_on` task are implicitly
        /// included)
        std::vector< std::reference_wrapper< const task > > run_after = {};
        /// Resources used by this task, only one task can access any resource at single point in
        /// time.
        std::vector< std::reference_wrapper< const resource > > resources = {};

        /// In case this is set to true, this task should not be visible in standard reports.
        bool hidden = false;
};

/// A set of tasks that contains either tasks or another sets. This forms a tree representing the
/// entire task set.
///
/// Convetion is that names inside the structure are concatenated to form a full path. Task `x` in
/// root set is referred as `//x`. Task `y` in subset `z` is referred as `//z/y`.
struct task_set
{
        /// Tasks of this set
        std::map< std::string, task > tasks;
        /// Subsets of this set
        std::map< std::string, task_set > sets;
};

/// Recursively executes function `f` for each task in set `ts`. Function is given full name of the
/// task and reference to the task. Respects qualifiers of the input task set.
template < typename T, typename Fun >
        requires( std::same_as< std::remove_cvref_t< T >, task_set > )
void for_each_task( T& ts, Fun&& f );

/// Recursively adds dependency on `dep` for each task in `ts`, except for `dep` itself.
void for_each_add_dep( task_set& ts, const task& dep );

/// Recursively adds dependency on each task in `ts` to task `t`, except for `t` itself.
void add_dep_to_each( task& t, const task_set& ts );

/// Recursively adds run after relationship so that all tasks in `ts` are run after `t`.
void run_each_after( task_set& ts, const task& t );

/// Recursively adds run after relationship so that task `t` is run after all tasks in set `ts`.
void run_after_all_of( task& t, const task_set& ts );

template < typename T, typename Fun >
void for_each_task_impl( T& ts, Fun&& f, const std::string& prefix )
{
        for ( auto& [name, t] : ts.tasks ) {
                f( ( prefix + "/" ).append( name ), t );
        }
        for ( auto& [name, s] : ts.sets ) {
                for_each_task_impl( s, f, ( prefix + "/" ).append( name ) );
        }
}

template < typename T, typename Fun >
        requires( std::same_as< std::remove_cvref_t< T >, task_set > )
void for_each_task( T& ts, Fun&& f )
{
        for_each_task_impl( ts, f, "/" );
}

}  // namespace joque
