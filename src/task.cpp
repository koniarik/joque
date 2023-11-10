#include "joque/task.hpp"

namespace joque
{
void for_each_add_dep( task_set& ts, const task& dep )
{
        for_each_task( ts, [&]( const std::string&, task& t ) {
                if ( &t != &dep ) {
                        t.depends_on.push_back( &dep );
                }
        } );
}

void add_dep_to_each( task& t, const task_set& ts )
{
        for_each_task( ts, [&]( const std::string&, const task& other ) {
                if ( &t != &other ) {
                        t.depends_on.push_back( &other );
                }
        } );
}

void run_after_all_of( task& t, const task_set& ts )
{
        for_each_task( ts, [&]( const std::string&, const task& other ) {
                if ( &t != &other ) {
                        t.run_after.push_back( &other );
                }
        } );
}
}  // namespace joque
