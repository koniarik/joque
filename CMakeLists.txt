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

add_library(
  joque STATIC
  src/dag.cpp
  src/exec.cpp
  src/exec_coro.cpp
  src/format.cpp
  src/format_dag.cpp
  src/junit.cpp
  src/print_exec_visitor.cpp
  src/process.cpp
  src/task.cpp)
target_link_libraries(joque PUBLIC reproc++)
target_include_directories(joque PUBLIC include)
target_compile_options(
  joque
  PRIVATE -Wall
          -Wextra
          -Wpedantic
          -Wnon-virtual-dtor
          -Wunused
          -Wconversion
          -Wno-missing-field-initializers)

include(CTest)
if(BUILD_TESTING)
  find_package(GTest REQUIRED)

  add_executable(joque_basic_utest test/basic_utest.cpp)
  add_test(NAME joque_basic COMMAND joque_basic_utest)
  target_link_libraries(joque_basic_utest PUBLIC joque GTest::GTest GTest::Main)

  add_executable(joque_process_utest test/process_utest.cpp)
  add_test(NAME joque_process COMMAND joque_process_utest)
  target_link_libraries(joque_process_utest PUBLIC joque GTest::GTest
                                                   GTest::Main)
  add_executable(joque_list_utest test/list_utest.cpp)
  add_test(NAME joque_list COMMAND joque_list_utest)
  target_link_libraries(joque_list_utest PUBLIC joque GTest::GTest GTest::Main)

  add_executable(joque_list_ptr_utest test/list_ptr_utest.cpp)
  add_test(NAME joque_list_ptr COMMAND joque_list_ptr_utest)
  target_link_libraries(joque_list_ptr_utest PUBLIC joque GTest::GTest
                                                    GTest::Main)
endif()
