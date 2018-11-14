/* -*- mode: c++; c-file-style: "bsd"; c++-basic-offset: 4; indent-tabs-mode nil -*- */
/***************************************************************************************************

Project libmodule
Copyright 2018 Sergey Ushakov

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software distributed under the License is
distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
implied.  See the License for the specific language governing permissions and limitations under the
License.
***************************************************************************************************/

#include <libmodules/receiver.hpp>
#include <gmock/gmock.h>

using namespace mtl;
using namespace std;

struct test_receiver_signals
{};

TEST(receiver, can_get_receiver)
{
    receiver<test_receiver_signals> r;
    EXPECT_EQ(&static_cast<test_receiver_signals&>(r), r.get());
}

TEST(receiver, copy_constructors_does_not_copy_transmitters)
{
    receiver<test_receiver_signals> r;
    receiver<test_receiver_signals> r_copy(r);
    EXPECT_EQ(&static_cast<test_receiver_signals&>(r_copy), r_copy.get());
    receiver<test_receiver_signals> r_move(move(r));
    EXPECT_EQ(&static_cast<test_receiver_signals&>(r_move), r_move.get());
}

TEST(receiver, copy_operators_does_not_copy_transmitters)
{
    receiver<test_receiver_signals> r;
    receiver<test_receiver_signals> r_copy = r;
    EXPECT_EQ(&static_cast<test_receiver_signals&>(r_copy), r_copy.get());
    receiver<test_receiver_signals> r_move = move(r);
    EXPECT_EQ(&static_cast<test_receiver_signals&>(r_move), r_move.get());
}
