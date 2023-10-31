#pragma once

#include "joque/job.hpp"

#include <map>

namespace joque
{

struct resource
{
        std::string name;
};

struct task
{
        job_ptr                        job;
        bool                           hidden    = false;
        std::vector< const task* >     deps      = {};
        std::vector< const task* >     after     = {};
        std::vector< const resource* > resources = {};
};

struct task_set
{
        using tasks_container = std::map< std::string, task >;
        using sets_container  = std::map< std::string, task_set >;

        tasks_container tasks;
        sets_container  sets;
};

template < typename T, typename Fun >
        requires( std::same_as< std::remove_cvref_t< T >, task_set > )
void for_each_task( T& ts, Fun&& f, const std::string& prefix = "/" )
{
        for ( auto& [name, t] : ts.tasks ) {
                f( ( prefix + "/" ).append( name ), t );
        }
        for ( auto& [name, s] : ts.sets ) {
                for_each_task( s, f, ( prefix + "/" ).append( name ) );
        }
}

inline void for_each_add_dep( task_set& ts, const task& dep )
{
        for_each_task( ts, [&]( const std::string&, task& t ) {
                if ( &t != &dep ) {
                        t.deps.push_back( &dep );
                }
        } );
}

inline void add_dep_to_each( task& t, const task_set& ts )
{
        for_each_task( ts, [&]( const std::string&, const task& other ) {
                if ( &t != &other ) {
                        t.deps.push_back( &other );
                }
        } );
}

inline void after_all_of( task& t, const task_set& ts )
{
        for_each_task( ts, [&]( const std::string&, const task& other ) {
                if ( &t != &other ) {
                        t.after.push_back( &other );
                }
        } );
}

}  // namespace joque
