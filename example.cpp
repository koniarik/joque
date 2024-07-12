
#include <joque/exec.hpp>

joque::task_set gen_tasks( std::size_t n )
{
        joque::task_set res;
        for ( std::size_t i = 0; i < n; i++ ) {
                res.tasks["task_" + std::to_string( i )] = joque::task{
                    .job = []( const joque::task& ) -> joque::run_result {
                            return { 0 };
                    },
                };
        }
        return res;
}

int main( int argc, char* argv[] )
{
        joque::task_set ts;
        ts.sets["a"] = gen_tasks( 8 );
        ts.sets["b"] = gen_tasks( 8 );
        joque::exec( ts ).run();
}