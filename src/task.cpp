#include "joque/task.hpp"

namespace joque
{
void for_each_add_dep( task_set& ts, const task_iface& dep )
{
        for_each_task( ts, [&]( const std::string&, task_iface& t ) {
                if ( &t != &dep )
                        t.add_dependency( dep );
        } );
}

void add_dep_to_each( task_iface& t, const task_set& ts )
{
        for_each_task( ts, [&]( const std::string&, const task_iface& other ) {
                if ( &t != &other )
                        t.add_dependency( other );
        } );
}

void run_each_after( task_set& ts, const task_iface& t )
{
        for_each_task( ts, [&]( const std::string&, task_iface& other ) {
                if ( &other != &t )
                        other.add_run_after( t );
        } );
}

void run_after_all_of( task_iface& t, const task_set& ts )
{
        for_each_task( ts, [&]( const std::string&, const task_iface& other ) {
                if ( &t != &other )
                        t.add_run_after( other );
        } );
}

}  // namespace joque
