cmake_minimum_required(VERSION 3.14)

project(joque)

option(JOQUE_TESTS_ENABLED "Run joque tests" OFF)
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
set(JOQUE_WARNINGS
    -Wall
    -Wextra
    -Wpedantic
    -Wnon-virtual-dtor
    -Wunused
    -Wconversion
    -Wno-missing-field-initializers)
target_compile_options(joque PRIVATE ${JOQUE_WARNINGS})

include(FetchContent)

FetchContent_Declare(
  reproc
  GIT_REPOSITORY https://github.com/DaanDeMeyer/reproc.git
  GIT_TAG v14.2.5
  GIT_SHALLOW TRUE)
FetchContent_Declare(
  nlohmann_json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG v3.11.3
  GIT_SHALLOW TRUE)

if(JOQUE_REPROC_ENABLED)
  set(REPROC++ ON)

  FetchContent_MakeAvailable(reproc)

  add_library(joque_reproc STATIC src/process.cpp)
  target_compile_options(joque_reproc PRIVATE ${JOQUE_WARNINGS})
  target_compile_features(joque_reproc PRIVATE cxx_std_20)
  target_link_libraries(joque_reproc PUBLIC reproc++)
  target_include_directories(joque_reproc PUBLIC include/)
  target_link_libraries(joque PUBLIC joque_reproc)

endif()

if(JOQUE_NLOHMANN_JSON_ENABLED)
  FetchContent_MakeAvailable(nlohmann_json)

  add_library(joque_json STATIC src/json.cpp)
  target_compile_options(joque_json PRIVATE ${JOQUE_WARNINGS})
  target_compile_features(joque_json PRIVATE cxx_std_20)
  target_link_libraries(joque_json PUBLIC nlohmann_json::nlohmann_json)
  target_include_directories(joque_json PUBLIC include/)
  target_link_libraries(joque PUBLIC joque_json)

endif()

if(JOQUE_TESTS_ENABLED)
  if(NOT JOQUE_REPROC_ENABLED)
    message(FATAL_ERROR "JOQUE_TESTS_ENABLED requires JOQUE_REPROC_ENABLED")
  endif()

  include(CTest)
  if(BUILD_TESTING)
    add_subdirectory(test)
  endif()

  add_executable(example example.cpp)
  target_compile_options(example PRIVATE ${JOQUE_WARNINGS})
  target_compile_features(example PRIVATE cxx_std_20)
  target_link_libraries(example PUBLIC joque)

endif()
