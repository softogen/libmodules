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

#include <libmodules/proxy_receiver.hpp>
#include <libmodules/receiver.hpp>
#include <gmock/gmock.h>

using namespace mtl;
using namespace std;

struct test_proxy_receiver_signals
{
    virtual ~test_proxy_receiver_signals() {}
    bool received = false;

    virtual void some_signal() = 0;
};

struct test_simple_receiver
    : public receiver<test_proxy_receiver_signals>
{
    void some_signal() override { received = true; }
};

TEST(proxy_receiver, has_no_receiver)
{
    proxy_receiver<test_proxy_receiver_signals> p;
    EXPECT_EQ(nullptr, p.get());
}

TEST(proxy_receiver, can_transmit_signals)
{
    proxy_receiver<test_proxy_receiver_signals> p;
    test_simple_receiver r;
    p.attach(r);
    p.transmit_signal(packed_signal<test_proxy_receiver_signals>(bind(&test_proxy_receiver_signals::some_signal, placeholders::_1)));
    EXPECT_TRUE(r.received);
}
