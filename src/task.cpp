#include "joque/task.hpp"

#include <string>

namespace joque
{
void for_each_add_dep( task_set& ts, const task& dep )
{
        for_each_task( ts, [&]( const std::string&, task& t ) {
                if ( &t != &dep )
                        t.depends_on.emplace_back( dep );
        } );
}

void add_dep_to_each( task& t, const task_set& ts )
{
        for_each_task( ts, [&]( const std::string&, const task& other ) {
                if ( &t != &other )
                        t.depends_on.emplace_back( other );
        } );
}

void run_each_after( task_set& ts, const task& t )
{
        for_each_task( ts, [&]( const std::string&, task& other ) {
                if ( &other != &t )
                        other.run_after.emplace_back( t );
        } );
}

void run_after_all_of( task& t, const task_set& ts )
{
        for_each_task( ts, [&]( const std::string&, const task& other ) {
                if ( &t != &other )
                        t.run_after.emplace_back( other );
        } );
}

void invalidated_by_all_of( task& t, const task_set& ts )
{
        for_each_task( ts, [&]( const std::string&, const task& other ) {
                if ( &t != &other )
                        t.invalidated_by.emplace_back( other );
        } );
}

}  // namespace joque
