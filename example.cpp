
#include <joque/exec.hpp>

using namespace std::chrono_literals;

joque::task_set gen_tasks( std::size_t n )
{
        joque::task_set res;
        for ( std::size_t i = 0; i < n; i++ ) {
                res.tasks["task_" + std::to_string( i )] = joque::task{
                    .job = []( const joque::task& ) -> joque::run_result {
                            std::this_thread::sleep_for( 200ms );

                            return { 0 };
                    },
                };
        }
        return res;
}

int main( int argc, char* argv[] )
{
        if ( argc != 2 )
                return 1;
        joque::task_set ts;
        int             n = std::stoi( argv[1] );
        ts.sets["a"]      = gen_tasks( n );
        ts.sets["b"]      = gen_tasks( n );
        joque::exec( ts ).run();
}
