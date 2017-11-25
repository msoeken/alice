cmake_minimum_required(VERSION 3.1)
project(alice LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)

# Options
option(ALICE_EXAMPLES "Build examples" OFF)
option(ALICE_TEST "Build tests" OFF)

# clang specific compiler definitions
if ("${CMAKE_CXX_COMPILER_ID}" MATCHES ".*Clang$")
  add_definitions(-fcolor-diagnostics)
endif()

set(CMAKE_CXX_FLAGS "-Wall")

add_subdirectory(lib)
add_subdirectory(include)

if(ALICE_EXAMPLES)
  add_subdirectory(examples)
endif()

if(ALICE_TEST)
  add_subdirectory(test EXCLUDE_FROM_ALL)
endif()

