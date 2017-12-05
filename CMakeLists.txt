#
# Project libmodules
# Copyright 2017 Sergey Ushakov
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

cmake_minimum_required(VERSION 3.4)
project(libmodules)

set (CMAKE_CXX_STANDARD 11)

set (GTestDir "dependencies/gtest")
add_subdirectory("${GTestDir}")
set (GTEST_LIBRARY "${gmock_BINARY_DIR}/libgmock.a") # ${GTestDir}/googlemock/libgmock.a)
set (GTEST_MAIN_LIBRARY "${gmock_BINARY_DIR}/libgmock_main.a") #${GTestDir}/googlemock/libgmock_main.a)

find_package (Threads)
find_package(GTest)

add_library(libmodules INTERFACE)
target_include_directories(libmodules INTERFACE include)

# Setup testing
enable_testing()

# Add test cpp file
add_executable(libmodules_unit_test
    include/libmodules/lockable.hpp
    include/libmodules/spy_pointer.hpp
    test/spy_pointer_test.cpp
)

# Link test executable against gtest & gtest_main
target_link_libraries(libmodules_unit_test gmock gmock_main ${CMAKE_THREAD_LIBS_INIT} libmodules)
#add_test(NAME libmodules_unit_test COMMAND libmodules_unit_test)
gtest_add_tests(TARGET libmodules_unit_test)
