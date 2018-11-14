cmake_minimum_required(VERSION 3.4)
project(libmodules)

set (CMAKE_CXX_STANDARD 11)
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/dependencies/cmake-modules")
set(gtest_force_shared_crt ON)

add_definitions(-D_SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING)

set (GTestDir "${PROJECT_SOURCE_DIR}/dependencies/gtest")
add_subdirectory("${GTestDir}")
set (GTEST_INCLUDE_DIR  "${gmock_SOURCE_DIR}/include"
                        "${gmock_SOURCE_DIR}"
                        "${gtest_SOURCE_DIR}/include")
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
  include/libmodules/signal.hpp
  include/libmodules/emitter.hpp
  include/libmodules/receiver.hpp
  include/libmodules/proxy_receiver.hpp
  test/linked_list_test.cpp
  test/spy_pointer_test.cpp
  test/emitter_test.cpp
  test/receiver_test.cpp
  test/proxy_receiver_test.cpp
)

# Link test executable against gtest & gtest_main
target_link_libraries(libmodules_unit_test
  gtest
  gmock
  gmock_main
  ${CMAKE_THREAD_LIBS_INIT}
  libmodules
)

gtest_add_tests(libmodules_unit_test "" AUTO)

if(CMAKE_COMPILER_IS_GNUCXX)
    include(CodeCoverage)
    APPEND_COVERAGE_COMPILER_FLAGS()
    set(COVERAGE_LCOV_EXCLUDES '/usr/*' 'gtest/*')
    SETUP_TARGET_FOR_COVERAGE_LCOV(
        NAME ${PROJECT_NAME}_coverage
        EXECUTABLE libmodules_unit_test
        DEPENDENCIES libmodules_unit_test)
endif()
