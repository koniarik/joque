# Builds against an installed joque; point CMAKE_PREFIX_PATH at the install
# prefix.
cmake_minimum_required(VERSION 3.14)

project(joque_install_consumer CXX)

find_package(joque REQUIRED)

add_executable(consumer main.cpp)
target_link_libraries(consumer joque::joque)
