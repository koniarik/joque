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
target_compile_features(joque INTERFACE cxx_std_20)
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

if(JOQUE_REPROC_ENABLED)
  include(FetchContent)

  set(REPROC++ ON)
  FetchContent_Declare(
    reproc
    SYSTEM
    GIT_REPOSITORY https://github.com/DaanDeMeyer/reproc/
    GIT_TAG v14.2.4
    GIT_SHALLOW TRUE
    FIND_PACKAGE_ARGS NAMES reproc
    )
  FetchContent_MakeAvailable(reproc)
  target_compile_options(reproc++ PRIVATE -Wno-changes-meaning)

  add_library(joque_reproc STATIC src/process.cpp)
  target_compile_features(joque_reproc INTERFACE cxx_std_20)
  target_link_libraries(joque_reproc PUBLIC reproc++)
  target_include_directories(joque_reproc PUBLIC include/)
  target_link_libraries(joque PUBLIC joque_reproc)

endif()

if(JOQUE_NLOHMANN_JSON_ENABLED)
  include(FetchContent)

  FetchContent_Declare(
      nlohmann_json 
      SYSTEM
      GIT_REPOSITORY https://github.com/nlohmann/json.git
      GIT_TAG v3.11.3
      GIT_SHALLOW TRUE
      FIND_PACKAGE_ARGS NAMES nlohmann_json
  )
  FetchContent_MakeAvailable(nlohmann_json)

  add_library(joque_json STATIC src/json.cpp)
  target_compile_features(joque_json INTERFACE cxx_std_20)
  target_link_libraries(joque_json PUBLIC nlohmann_json::nlohmann_json)
  target_include_directories(joque_json PUBLIC include/)
  target_link_libraries(joque PUBLIC joque_json)

endif()

if(JOQUE_TESTS_ENABLED)

  include(CTest)
  if(BUILD_TESTING)
    find_package(GTest REQUIRED)

    function(add_joque_test test)
      add_executable(joque_${test} test/${test}.cpp)
      add_test(NAME joque_${test}_test COMMAND joque_${test})
      target_link_libraries(joque_${test} PUBLIC joque GTest::GTest GTest::Main)
    endfunction()

    add_joque_test(basic_utest)
    add_joque_test(process_utest)
    add_joque_test(format_utest)
    add_joque_test(list_utest)
    add_joque_test(list_ptr_utest)
  endif()

  add_executable(example example.cpp)
  target_link_libraries(example PUBLIC joque)

endif()
