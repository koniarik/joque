#pragma once

#include "joque/job.hpp"

#include <functional>
#include <map>
#include <span>

namespace joque
{

/// Abstraction to model resource used by tasks. Out of all tasks that reference single resource,
/// only one may be executed at any point in time.
struct resource
{
        std::string name;
};

struct task_iface;

using const_task_iface_ref = std::reference_wrapper< const task_iface >;
using const_resource_ref   = std::reference_wrapper< const resource >;

struct task_iface
{
        virtual const job_ptr&                          job() const                         = 0;
        virtual std::span< const const_task_iface_ref > depends_on() const                  = 0;
        virtual void                                    add_dependency( const task_iface& ) = 0;
        virtual std::span< const const_task_iface_ref > run_after() const                   = 0;
        virtual void                                    add_run_after( const task_iface& )  = 0;
        virtual std::span< const const_resource_ref >   resources() const                   = 0;
        virtual bool                                    is_hidden() const                   = 0;

        virtual ~task_iface() = default;
};

template < typename T >
struct task_traits;

template < typename T >
struct task_wrapper : task_iface
{
        T item;

        task_wrapper( T item )
          : item( std::move( item ) )
        {
        }

        const job_ptr& job() const override
        {
                return task_traits< T >::ptr( item );
        }
        std::span< const const_task_iface_ref > depends_on() const override
        {
                return task_traits< T >::depends_on( item );
        }
        void add_dependency( const task_iface& t ) override
        {
                return task_traits< T >::add_dependency( item, t );
        }
        std::span< const const_task_iface_ref > run_after() const override
        {
                return task_traits< T >::run_after( item );
        }
        void add_run_after( const task_iface& t ) override
        {
                return task_traits< T >::add_run_after( item, t );
        }
        std::span< const const_resource_ref > resources() const override
        {
                return task_traits< T >::resources( item );
        }
        bool is_hidden() const
        {
                return task_traits< T >::is_hidden( item );
        }
};

/// Single task that should be executed by the system.
struct task
{
        /// Job being executed for the task
        job_ptr job;

        /// Dependencies of the task - all of these should be executed before this task. In case any
        /// of these is invalidated, this task is also invalidated.
        std::vector< const_task_iface_ref > depends_on = {};
        /// Tasks that should be executed before this task. (`depends_on` task are implicitly
        /// included)
        std::vector< const_task_iface_ref > run_after = {};
        /// Resources used by this task, only one task can access any resource at single point in
        /// time.
        std::vector< const_resource_ref > resources = {};

        /// In case this is set to true, this task should not be visible in standard reports.
        bool hidden = false;
};

template <>
struct task_traits< task >
{
        static const job_ptr& ptr( const task& t )
        {
                return t.job;
        }
        static std::span< const const_task_iface_ref > depends_on( const task& t )
        {
                return t.depends_on;
        }
        static void add_dependency( task& t, const task_iface& dep )
        {
                t.depends_on.emplace_back( dep );
        }
        static std::span< const const_task_iface_ref > run_after( const task& t )
        {
                return t.run_after;
        }
        static void add_run_after( task& t, const task_iface& dep )
        {
                t.run_after.emplace_back( dep );
        }
        static std::span< const const_resource_ref > resources( const task& t )
        {
                return t.resources;
        }
        static bool is_hidden( const task& t )
        {
                return t.hidden;
        }
};

struct task_ptr : std::unique_ptr< task_iface >
{
        task_ptr() = default;

        template < typename T >
        task_ptr( T&& item )
          : std::unique_ptr< task_iface >(
                new task_wrapper< std::decay_t< T > >{ std::forward< T >( item ) } )
        {
        }

        task_ptr( const task_ptr& )            = delete;
        task_ptr& operator=( const task_ptr& ) = delete;

        task_ptr( task_ptr&& ) noexcept            = default;
        task_ptr& operator=( task_ptr&& ) noexcept = default;
};


/// A set of tasks that contains either tasks or another sets. This forms a tree representing the
/// entire task set.
///
/// Convetion is that names inside the structure are concatenated to form a full path. Task `x` in
/// root set is referred as `//x`. Task `y` in subset `z` is referred as `//z/y`.
struct task_set
{
        /// Tasks of this set
        std::map< std::string, task_ptr > tasks;
        /// Subsets of this set
        std::map< std::string, task_set > sets;
};

/// Recursively executes function `f` for each task in set `ts`. Function is given full name of the
/// task and reference to the task. Respects qualifiers of the input task set.
template < typename T, typename Fun >
        requires( std::same_as< std::remove_cvref_t< T >, task_set > )
void for_each_task( T& ts, Fun&& f );

/// Recursively adds dependency on `dep` for each task in `ts`, except for `dep` itself.
void for_each_add_dep( task_set& ts, const task_iface& dep );

/// Recursively adds dependency on each task in `ts` to task `t`, except for `t` itself.
void add_dep_to_each( task_iface& t, const task_set& ts );

/// Recursively adds run after relationship so that all tasks in `ts` are run after `t`.
void run_each_after( task_set& ts, const task_iface& t );

/// Recursively adds run after relationship so that task `t` is run after all tasks in set `ts`.
void run_after_all_of( task_iface& t, const task_set& ts );

template < typename T, typename Fun >
void for_each_task_impl( T& ts, Fun&& f, const std::string& prefix )
{
        for ( auto& [name, t] : ts.tasks )
                f( ( prefix + "/" ).append( name ), *t );
        for ( auto& [name, s] : ts.sets )
                for_each_task_impl( s, f, ( prefix + "/" ).append( name ) );
}

template < typename T, typename Fun >
        requires( std::same_as< std::remove_cvref_t< T >, task_set > )
void for_each_task( T& ts, Fun&& f )
{
        for_each_task_impl( ts, f, "/" );
}

}  // namespace joque
