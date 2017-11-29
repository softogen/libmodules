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
find_package (Threads)

add_library(libmodules INTERFACE)
target_include_directories(libmodules INTERFACE ../include)

# Setup testing
enable_testing()

add_subdirectory(../dependencies/gtest dependencies/gtest)

# Add test cpp file
add_executable(libmodules_unit_test
    ../include/libmodules/locable.hpp
    ../include/libmodules/weak_pointer.hpp
    ../test/weak_pointer_test.cpp
)

# Link test executable against gtest & gtest_main
target_link_libraries(libmodules_unit_test gtest gtest_main ${CMAKE_THREAD_LIBS_INIT} libmodules)
add_test(NAME libmodules_unit_test COMMAND libmodule_unit_test)
