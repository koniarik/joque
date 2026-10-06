FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG release-1.12.1
  GIT_SHALLOW TRUE)
set(INSTALL_GTEST
    OFF
    CACHE BOOL "")
set(gtest_force_shared_crt
    ON
    CACHE BOOL "")
FetchContent_MakeAvailable(nlohmann_json googletest)

file(GLOB TESTS *.cpp)
add_executable(joque_utest ${TESTS})
target_compile_options(joque_utest PRIVATE ${JOQUE_WARNINGS})
add_test(NAME joque_utest COMMAND joque_utest)
target_compile_features(joque_utest PRIVATE cxx_std_20)
target_link_libraries(joque_utest PUBLIC joque gtest gtest_main)
