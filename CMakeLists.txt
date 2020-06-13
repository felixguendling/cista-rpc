cmake_minimum_required(VERSION 3.15)
project(crpc)

################################
# Dependencies
################################
#include(cmake/pkg.cmake)
add_subdirectory(deps)


################################
# Linter
################################
option(CRPC_LINT "enable lint (clang-tidy) target" OFF)
if (CRPC_LINT)
  include(cmake/clang-tidy.cmake)
endif ()


################################
# cprc Library
################################
add_library(crpc INTERFACE)
target_link_libraries(crpc INTERFACE cista ctx ws-client web-server)
target_include_directories(crpc INTERFACE include)
target_compile_features(crpc INTERFACE cxx_std_17)


################################
# Tests
################################
file(GLOB_RECURSE crpc-src-test-files test/*.cc)
add_executable(crpc-test ${crpc-src-test-files})
target_link_libraries(crpc-test doctest crpc)


################################
# Example
################################
add_executable(crpc-example example.cc)
target_link_libraries(crpc-example crpc)

