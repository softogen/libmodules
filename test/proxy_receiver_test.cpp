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

#include <memory>

using namespace mtl;
using namespace std;

struct test_filter_proxy_receiver;
struct test_proxy_receiver_signals
{
    virtual ~test_proxy_receiver_signals() {}
    bool received = false;

    virtual void some_signal(unique_ptr<test_filter_proxy_receiver>* obj = nullptr) = 0;
};

struct test_simple_receiver
    : public receiver<test_proxy_receiver_signals>
{
    void some_signal(unique_ptr<test_filter_proxy_receiver>* obj) override
    {
        received = true;
        ASSERT_TRUE(!obj);
    }
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
    p.transmit_signal(packed_signal<test_proxy_receiver_signals>(bind(&test_proxy_receiver_signals::some_signal, placeholders::_1, nullptr)));
    EXPECT_TRUE(r.received);
}

struct test_filter_proxy_receiver
    : public filter_proxy_receiver<test_proxy_receiver_signals>
{
    bool do_filtering = false;

    void some_signal(unique_ptr<test_filter_proxy_receiver>* obj) override
    {
        received = true;
        if (do_filtering)
            this->filter_signal();

        if (obj)
            obj->reset();
    }
};

TEST(filter_proxy_receiver, can_perform_filter_signal)
{
    test_filter_proxy_receiver f;
    f.transmit_signal(packed_signal<test_proxy_receiver_signals>(bind(&test_proxy_receiver_signals::some_signal, placeholders::_1, nullptr)));
    EXPECT_TRUE(f.received);
}

TEST(filter_proxy_receiver, can_pass_signal)
{
    test_filter_proxy_receiver f;
    test_simple_receiver r;
    f.attach(r);
    f.transmit_signal(packed_signal<test_proxy_receiver_signals>(bind(&test_proxy_receiver_signals::some_signal, placeholders::_1, nullptr)));
    EXPECT_TRUE(r.received);
}

TEST(filter_proxy_receiver, can_filter_signal)
{
    test_filter_proxy_receiver f;
    f.do_filtering = true;

    test_simple_receiver r;
    f.attach(r);
    f.transmit_signal(packed_signal<test_proxy_receiver_signals>(bind(&test_proxy_receiver_signals::some_signal, placeholders::_1, nullptr)));
    EXPECT_FALSE(r.received);
}

TEST(filter_proxy_receiver, can_be_destroyed_by_filter)
{
    unique_ptr<test_filter_proxy_receiver> my_obj(new test_filter_proxy_receiver());
    test_simple_receiver r;
    my_object->attach(r);
    EXPECT_NO_FATAL_FAILURE( my_object->transmit_signal( packed_signal< test_proxy_receiver_signals >( std::bind( &test_proxy_receiver_signals::some_signal, std::placeholders::_1, &obj ))));
    EXPECT_FALSE(r.received);
}

TEST(queue_proxy_receiver, can_pop_empty_queue)
{
    queue_proxy_receiver<test_proxy_receiver_signals> q;
    test_simple_receiver r;
    q.attach(r);

    EXPECT_FALSE(q.pop_signal());
    EXPECT_FALSE(r.received);
}

TEST(queue_proxy_receiver, can_queue_signal)
{
    queue_proxy_receiver<test_proxy_receiver_signals> q;
    test_simple_receiver r;
    q.attach(r);

    q.transmit_signal(packed_signal<test_proxy_receiver_signals>(bind(&test_proxy_receiver_signals::some_signal, placeholders::_1, nullptr)));
    EXPECT_FALSE(r.received);

    EXPECT_TRUE(q.pop_signal());
    EXPECT_TRUE(r.received);
}

struct test_locker
{
    int* _lock_count = nullptr;
    bool* _was_locked = nullptr;

    test_locker(int* lock_count, bool* was_locked) : _lock_count(lock_count), _was_locked(was_locked) {}
    test_locker(const test_locker& other) = default;
    test_locker(test_locker&& other) = default;
    void lock()
    {
        *_was_locked = true;
        ++*_lock_count;
    }
    void unlock()
    {
        --*_lock_count;
        EXPECT_LE(0, *_lock_count);
    }
};

TEST(queue_proxy_receiver, can_lock_queue)
{
    bool was_locked = false;
    int lock_count = 0;

    queue_proxy_receiver<test_proxy_receiver_signals, test_locker> q(test_locker(&lock_count, &was_locked));
    q.transmit_signal(packed_signal<test_proxy_receiver_signals>(bind(&test_proxy_receiver_signals::some_signal, placeholders::_1, nullptr)));
    EXPECT_TRUE(was_locked);
    EXPECT_EQ(0, lock_count);

    was_locked = false;
    lock_count = 0;
    EXPECT_TRUE(q.pop_signal());
    EXPECT_TRUE(was_locked);
    EXPECT_EQ(0, lock_count);
}

TEST(queue_proxy_receiver, can_copy_locker)
{
    bool was_locked = false;
    int lock_count = 0;

    queue_proxy_receiver<test_proxy_receiver_signals, test_locker> q1(test_locker(&lock_count, &was_locked));
    queue_proxy_receiver<test_proxy_receiver_signals, test_locker> q2(q1);
    q2.transmit_signal(packed_signal<test_proxy_receiver_signals>(bind(&test_proxy_receiver_signals::some_signal, placeholders::_1, nullptr)));
    EXPECT_TRUE(was_locked);
    EXPECT_EQ(0, lock_count);
}

TEST(queue_proxy_receiver, can_move_locker)
{
    bool was_locked = false;
    int lock_count = 0;

    queue_proxy_receiver<test_proxy_receiver_signals, test_locker> q1(test_locker(&lock_count, &was_locked));
    queue_proxy_receiver<test_proxy_receiver_signals, test_locker> q2(move(q1));
    q2.transmit_signal(packed_signal<test_proxy_receiver_signals>(bind(&test_proxy_receiver_signals::some_signal, placeholders::_1, nullptr)));
    EXPECT_TRUE(was_locked);
    EXPECT_EQ(0, lock_count);
}
