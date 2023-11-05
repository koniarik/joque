cmake_minimum_required(VERSION 3.12)

project(joque)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT TARGET reproc++)
  include(FetchContent)
  set(REPROC++ ON)
  FetchContent_Declare(
    reproc
    SYSTEM
    GIT_REPOSITORY https://github.com/DaanDeMeyer/reproc/
    GIT_TAG v14.2.4
    GIT_SHALLOW TRUE)
  FetchContent_MakeAvailable(reproc)
  target_compile_options(reproc++ PRIVATE -Wno-changes-meaning)
endif()

add_library(joque STATIC src/exec.cpp src/dag.cpp src/junit.cpp src/process.cpp
                         src/task.cpp src/exec_coro.cpp)
target_link_libraries(joque PUBLIC reproc++)
target_include_directories(joque PUBLIC include)

find_package(GTest REQUIRED)

include(CTest)
add_executable(joque_basic_utest test/basic_utest.cpp)
add_test(joque_basic joque_basic_utest)
target_link_libraries(joque_basic_utest PUBLIC joque GTest::GTest gtest_main)
