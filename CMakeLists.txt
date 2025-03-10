cmake_minimum_required(VERSION 3.12)

project(joque)

option(JOQUE_TESTS_ENABLED "Run joque tests" ON)
option(JOQUE_NLOHMANN_JSON_ENABLED "Enable nlohmann json" OFF)
option(JOQUE_REPROC_ENABLED "Enable reproc++" OFF)

add_library(
  joque STATIC
  src/dag.cpp
  src/exec.cpp
  src/exec_coro.cpp
  src/format.cpp
  src/format_dag.cpp
  src/junit.cpp
  src/records.cpp
  src/print_exec_visitor.cpp
  src/task.cpp)
target_compile_features(joque PRIVATE cxx_std_20)
target_include_directories(joque PUBLIC include/)
target_compile_options(
  joque
  PRIVATE -Wall
          -Wextra
          -Wpedantic
          -Wnon-virtual-dtor
          -Wunused
          -Wconversion
          -Wno-missing-field-initializers)

include(cmake/CPM.cmake)

if(JOQUE_REPROC_ENABLED)
  set(REPROC++ ON)

  cpmaddpackage("gh:DaanDeMeyer/reproc@14.2.5")

  add_library(joque_reproc STATIC src/process.cpp)
  target_compile_features(joque_reproc PRIVATE cxx_std_20)
  target_link_libraries(joque_reproc PUBLIC reproc++)
  target_include_directories(joque_reproc PUBLIC include/)
  target_link_libraries(joque PUBLIC joque_reproc)

endif()

if(JOQUE_NLOHMANN_JSON_ENABLED)
  cpmaddpackage("gh:nlohmann/json@3.11.3")

  add_library(joque_json STATIC src/json.cpp)
  target_compile_features(joque_json PRIVATE cxx_std_20)
  target_link_libraries(joque_json PUBLIC nlohmann_json::nlohmann_json)
  target_include_directories(joque_json PUBLIC include/)
  target_link_libraries(joque PUBLIC joque_json)

endif()

if(JOQUE_TESTS_ENABLED)

  include(CTest)
  if(BUILD_TESTING)
    cpmaddpackage("gh:nlohmann/json@3.10.5")
    find_package(GTest REQUIRED)

    function(add_joque_test test)
      add_executable(joque_${test} test/${test}.cpp)
      add_test(NAME joque_${test}_test COMMAND joque_${test})
      target_compile_features(joque_${test} PRIVATE cxx_std_20)
      target_link_libraries(joque_${test} PUBLIC joque gtest GTest::Main)
    endfunction()

    add_joque_test(basic_utest)
    add_joque_test(process_utest)
    add_joque_test(format_utest)
    add_joque_test(list_utest)
    add_joque_test(list_ptr_utest)
  endif()

  add_executable(example example.cpp)
  target_compile_features(example PRIVATE cxx_std_20)
  target_link_libraries(example PUBLIC joque)

endif()
