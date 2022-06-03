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

#include <libmodules/emitter.hpp>
#include <gmock/gmock.h>

#include <memory>

using namespace mtl;
using namespace std;

struct test_receiver;

struct test_emitter_signals
{
    virtual ~test_emitter_signals() {}
    bool received = false;

    virtual void some_signal() = 0;
    virtual void attach(emitter<test_emitter_signals>& em, test_receiver& r) = 0;
    virtual void detach(emitter<test_emitter_signals>& em, test_receiver& r) = 0;
    virtual void detach_all(emitter<test_emitter_signals>& em) = 0;
    virtual void delete_obj(shared_ptr<void>& obj) = 0;
    virtual void throw_at(size_t& count_down) = 0;
    virtual void send_signal(emitter<test_emitter_signals>& em, packed_signal<test_emitter_signals> s) = 0;
};

struct test_receiver
    : public test_emitter_signals
    , public transmitter<test_emitter_signals>
{
    enum receiver_flavor
    {
        valid_receiver,
        invalid_receiver,
    };

    test_receiver(receiver_flavor flavor = valid_receiver) : transmitter<test_emitter_signals>(flavor == invalid_receiver ? nullptr : this) {}

    void some_signal() override                                               { received = true; }
    void attach(emitter<test_emitter_signals>& em, test_receiver& r) override { received = true; if(r.empty()) em.attach(r); } // Recursion is possible here
    void detach(emitter<test_emitter_signals>& em, test_receiver& r) override { received = true; em.detach(r); }
    void detach_all(emitter<test_emitter_signals>& em) override               { received = true; em.reset(); }
    void delete_obj(shared_ptr<void>& obj) override                           { received = true; obj.reset(); }
    void throw_at(size_t& count_down) override                                { received = true; if (!--count_down) throw runtime_error("Something bad happened."); }
    void send_signal(emitter<test_emitter_signals>& em, packed_signal<test_emitter_signals> s) override 
                                                                              { em.send(s); }
};

struct test_proxy
    : public transmitter<test_emitter_signals>
{
    virtual void transmit_signal(const packed_signal<test_emitter_signals>& call) { call(_receiver); }
    
    test_receiver _receiver;
};

TEST(emitting, can_attach_and_detach_receivers)
{
    emitter<test_emitter_signals> em;
    test_receiver r1, r2;
    EXPECT_TRUE(em.empty());
    EXPECT_TRUE(r1.empty());
    EXPECT_TRUE(r2.empty());

    em.attach(r1);
    EXPECT_FALSE(em.empty());
    EXPECT_FALSE(r1.empty());
    EXPECT_TRUE(r2.empty());

    em.attach(r2);
    EXPECT_FALSE(em.empty());
    EXPECT_FALSE(r1.empty());
    EXPECT_FALSE(r2.empty());

    em.detach(r1);
    EXPECT_FALSE(em.empty());
    EXPECT_TRUE(r1.empty());
    EXPECT_FALSE(r2.empty());

    em.detach(r2);
    EXPECT_TRUE(em.empty());
    EXPECT_TRUE(r1.empty());
    EXPECT_TRUE(r2.empty());
}

TEST(emitting, emitter_copies_attachments)
{
    emitter<test_emitter_signals> em1;
    test_receiver r;

    em1.attach(r);
    emitter<test_emitter_signals> em2(em1);
    em1.detach(r);
    EXPECT_FALSE(em2.empty());
    EXPECT_FALSE(r.empty());

    em1 = em2;
    em2.detach(r);
    EXPECT_FALSE(em1.empty());
    EXPECT_FALSE(r.empty());
}

TEST(emitting, emitter_moves_attachments)
{
    emitter<test_emitter_signals> em1;
    test_receiver r;

    em1.attach(r);
    emitter<test_emitter_signals> em2(move(em1));
    EXPECT_TRUE(em1.empty());
    EXPECT_FALSE(em2.empty());
    EXPECT_FALSE(r.empty());

    em1 = move(em2);
    EXPECT_TRUE(em2.empty());
    EXPECT_FALSE(em1.empty());
    EXPECT_FALSE(r.empty());
}

TEST(emitting, emitter_can_autodetach)
{
    test_receiver r;
    {
        emitter<test_emitter_signals> em;
        em.attach(r);
    }
    EXPECT_TRUE(r.empty());
}

TEST(emitting, receiver_can_autodetach)
{
    emitter<test_emitter_signals> em;
    {
        test_receiver r;
        em.attach(r);
    }
    EXPECT_TRUE(em.empty());
}

TEST(emitting, emitter_can_detach_all)
{
    emitter<test_emitter_signals> em;
    test_receiver r1, r2;

    em.attach(r1);
    em.attach(r2);
    em.reset();
    EXPECT_TRUE(em.empty());
    EXPECT_TRUE(r1.empty());
    EXPECT_TRUE(r2.empty());
}

TEST(emitting, receiver_can_detach_all)
{
    emitter<test_emitter_signals> em1, em2;
    test_receiver r;

    em1.attach(r);
    em2.attach(r);
    r.reset();
    EXPECT_TRUE(em1.empty());
    EXPECT_TRUE(em2.empty());
    EXPECT_TRUE(r.empty());
}

TEST(emitting, can_send_signals)
{
    emitter<test_emitter_signals> em;
    test_receiver r;

    em.attach(r);
    EXPECT_TRUE(em.send(&test_emitter_signals::some_signal));
    EXPECT_TRUE(r.received);
}

TEST(emitting, cna_forward_signals)
{
    emitter<test_emitter_signals> em;
    test_receiver r;

    em.attach(r);
    EXPECT_TRUE(em.send(packed_signal<test_emitter_signals>(bind(&test_emitter_signals::some_signal, placeholders::_1))));
    EXPECT_TRUE(r.received);
}

TEST(emitting, forwarding_nothing_is_not_an_error)
{
    emitter<test_emitter_signals> em;
    test_receiver r;

    em.attach(r);
    EXPECT_NO_THROW(em.send(packed_signal<test_emitter_signals>()));
    EXPECT_FALSE(r.received);
}

TEST(emitting, unable_forwardind_by_invalid_transmitter)
{
    emitter<test_emitter_signals> em;
	test_receiver r{ test_receiver::invalid_receiver };

    em.attach(r);
    EXPECT_THROW(em.send(packed_signal<test_emitter_signals>(bind(&test_emitter_signals::some_signal, placeholders::_1))), invalid_transmitter);
    EXPECT_FALSE(r.received);
}

TEST(emitting, can_forward_signals_through_proxy)
{
    emitter<test_emitter_signals> em;
    test_proxy p;

    em.attach(p);
    EXPECT_TRUE(em.send(&test_emitter_signals::some_signal));
    EXPECT_TRUE(p._receiver.received);
}

TEST(emitting, can_attach_while_sending)
{
    emitter<test_emitter_signals> em;
    test_receiver r1, r2;
    
    em.attach(r1);
    EXPECT_TRUE(em.send(&test_emitter_signals::attach, ref(em), ref(r2)));
    EXPECT_FALSE(r2.empty());
}

TEST(emitting, attacment_signal_will_not_be_transmitted_to_attached_receivers)
{
    emitter<test_emitter_signals> em;
    test_receiver r1, r2;

    em.attach(r1);
    EXPECT_TRUE(em.send(&test_emitter_signals::attach, ref(em), ref(r2)));
    EXPECT_FALSE(r2.received);
}

TEST(emitting, can_attach_while_forwarding)
{
    emitter<test_emitter_signals> em;
    test_receiver r1, r2;
    
    em.attach(r1);
    EXPECT_TRUE(em.send(packed_signal<test_emitter_signals>(bind(&test_emitter_signals::attach, placeholders::_1, ref(em), ref(r2)))));
    EXPECT_FALSE(r2.empty());
}

TEST(emitting, can_detach_while_sending)
{
    emitter<test_emitter_signals> em;
    test_receiver r1, r2;
    
    em.attach(r1);
    em.attach(r2);
    EXPECT_TRUE(em.send(&test_emitter_signals::detach, ref(em), ref(r2)));
    EXPECT_TRUE(r2.empty());
    EXPECT_FALSE(r2.received);
}

TEST(emitting, can_detach_while_forwarding)
{
    emitter<test_emitter_signals> em;
    test_receiver r1, r2;
    
    em.attach(r1);
    em.attach(r2);
    EXPECT_TRUE(em.send(packed_signal<test_emitter_signals>(bind(&test_emitter_signals::detach, placeholders::_1, ref(em), ref(r2)))));
    EXPECT_TRUE(r2.empty());
    EXPECT_FALSE(r2.received);
}

TEST(emitting, emitter_can_detach_all_while_sending)
{
    emitter<test_emitter_signals> em;
    test_receiver r1, r2;

    em.attach(r1);
    em.attach(r2);
    EXPECT_TRUE(em.send(&test_emitter_signals::detach_all, ref(em)));
    EXPECT_TRUE(em.empty());
    EXPECT_TRUE(r1.empty());
    EXPECT_TRUE(r1.received);
    EXPECT_TRUE(r2.empty());
    EXPECT_FALSE(r2.received);
}

TEST(emitting, can_delete_receiver_while_sending)
{
    emitter<test_emitter_signals> em;
    test_receiver* r = new test_receiver();
    
    em.attach(*r);
    shared_ptr<void> obj = shared_ptr<test_receiver>(r);
    EXPECT_TRUE(em.send(&test_emitter_signals::delete_obj, ref(obj)));
    EXPECT_TRUE(em.empty());
}

TEST(emitting, can_delete_emitter_while_sending)
{
    emitter<test_emitter_signals>* em = new emitter<test_emitter_signals>();
    test_receiver r;
    
    em->attach(r);
    shared_ptr<void> obj = shared_ptr<emitter<test_emitter_signals>>(em);
    EXPECT_FALSE(em->send(&test_emitter_signals::delete_obj, ref(obj)));
    EXPECT_TRUE(r.empty());
}

TEST(emitting, can_delete_emitter_while_recursive_sending)
{
    emitter<test_emitter_signals>* em = new emitter<test_emitter_signals>();
    test_receiver r;
    
    em->attach(r);

    shared_ptr<void> obj = shared_ptr<emitter<test_emitter_signals>>(em);
    packed_signal<test_emitter_signals> signal(bind(&test_emitter_signals::delete_obj, placeholders::_1, ref(obj)));
    EXPECT_FALSE(em->send(&test_emitter_signals::send_signal, ref(*em), signal));
    EXPECT_TRUE(r.empty());
}

TEST(emitting, can_interrupt_sending_by_exception_throwing)
{
    emitter<test_emitter_signals> em;
    test_receiver r1, r2;
    size_t counter = 1;
    
    em.attach(r1);
    em.attach(r2);
    EXPECT_THROW(em.send(&test_emitter_signals::throw_at, ref(counter)), runtime_error);
    EXPECT_TRUE(r1.received);
    EXPECT_FALSE(r2.received);
}

TEST(emitting, can_interrupt_recursive_sending_by_exception_throwing)
{
    emitter<test_emitter_signals> em;
    test_receiver r1, r2;
    size_t counter = 1;
    
    em.attach(r1);
    em.attach(r2);
    EXPECT_THROW(em.send(&test_emitter_signals::send_signal, ref(em), packed_signal<test_emitter_signals>(bind(&test_emitter_signals::throw_at, placeholders::_1, ref(counter)))), runtime_error);
    EXPECT_TRUE(r1.received);
    EXPECT_FALSE(r2.received);
}
