
# joque

C++20 library for ordered parallel execution of tasks.

Library implements similar behavior to `make` or `ninja`.
User defines tasks that shall be processed and library takes care of executing those tasks in lazy manner, with respect to dependencies.

More details on API in [Documentation](https://koniarik.github.io/joque/index.html)

## Install

Recommended way of installing is to use CMakes FetchContent, or use git submodules.

```cmake
FetchContent_Declare(
  joque
  GIT_REPOSITORY https://github.com/koniarik/joque
  GIT_TAG v0.1
)
FetchContent_MakeAvailable(joque)
```

## Description

Library gives users ability to define a set of tasks. The set is represented by a tree - each set may contain tasks or another subsets.

Each task or set has a name, and these are composed into a full path string.
For set M, which contains task A, full name of the task is `//M/A`.

`task` has attributes that defines it's dependencies, run after relationships, and resources that it needs. The rules are as follows:
 - if task A depends on task B, than A has to be executed after B, and if B is invalidated, so is A
 - if task A should run after B, than A has to be executed after B
 - if tasks A and B relies on resource X, than only one of those can be executed at once

Each `task` also defines what should be done for it's execution and how it becames invalidated. This is represented by it's `job` attribute. Library supports two types of jobs:
 - Callables, these are always invalidated, library just executes the callable object
 - `process`, represents a subprocess that shall be executed, it becames invalidated if some of input files are more recent than output files.

The set of tasks is given to `exec` function which builds internal DAG representing the task set and executes the tasks in parallel, respecting dependencies and resource management.

## Example

```cpp

#include "joque/exec.hpp"
#include "joque/task.hpp"

joque::task_set ts;

std::filesystem::path input_filename{ "in.txt" };
std::filesystem::path output_filename{ "out.txt" };

// create task with process
ts.tasks["A"] = joque::task{
    .job =
        joque::process{
            .cmd =
                { "my_script.sh",
                  "--in",
                  input_filename.string(),
                  "--out",
                  output_filename.string() },
            .input  = { input_filename },
            .output = { output_filename },
        },
    .hidden = true,  // won't be visible in output
};

// use smart deduction for process task
ts.tasks["B"] = joque::task{
    .job = joque::process::derive(
        "my_script.sh",
        "--in",
        input_filename,
        "--out",
        joque::out( output_filename ) ),
    .depends_on = { ts["A"] },
};

// create task with lambda function
ts.tasks["A"] = joque::task{
    .job =
        []( const joque::task& ) {
                std::cout << "A is running" << std::endl;
        },
    .depends_on = { ts["B"] },
};

// execute the tasks with one worker thread
joque::exec( ts, 1 );
```
