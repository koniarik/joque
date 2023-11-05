
# joque

C++20 library for ordered pararell execution of tasks.

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

