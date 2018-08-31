cmake_minimum_required(VERSION 3.4)
project(libmodules)

set (CMAKE_CXX_STANDARD 11)

set (GTestDir "dependencies/gtest")
add_subdirectory("${GTestDir}")
set (GTEST_LIBRARY      "${gmock_BINARY_DIR}/libgmock.a")
set (GTEST_MAIN_LIBRARY "${gmock_BINARY_DIR}/libgmock_main.a")

find_package (Threads)
find_package(GTest)

add_library(libmodules INTERFACE
)
target_include_directories(libmodules INTERFACE include)

# Setup testing
enable_testing()

# Add test cpp file
add_executable(libmodules_unit_test
  include/libmodules/lockable.hpp
  include/libmodules/linked_list.hpp
  include/libmodules/spy_pointer.hpp
  include/libmodules/emitter.hpp
  test/linked_list_test.cpp
  test/spy_pointer_test.cpp
  test/emitter_test.cpp
)

# Link test executable against gtest & gtest_main
target_link_libraries(libmodules_unit_test
  gtest
  gmock
  gmock_main
  ${CMAKE_THREAD_LIBS_INIT}
  libmodules
)
gtest_add_tests(TARGET libmodules_unit_test)
