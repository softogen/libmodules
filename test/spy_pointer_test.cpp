/***************************************************************************************************

Project libmodule
Copyright 2017 Sergey Ushakov

Licensed under the Apache License, Version 2.0 (the "License");
yesou may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software distributed under the License is
distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
implied.  See the License for the specific language governing permissions and limitations under the
License.
***************************************************************************************************/

#include <libmodules/spy_pointer.hpp>
#include <gmock/gmock.h>

using namespace mtl;

struct SpiedObject
    : public enable_spying
{
    bool spied_state_in_callback = false;
    virtual void foo() {}
    virtual void on_spying_state_changed() { spied_state_in_callback = is_spied(); }
};

struct ChildObject
    : public SpiedObject
{
    bool state = false;
};

TEST(spying, can_dereference_spy)
{
    ChildObject obj;
    spy_pointer<ChildObject> spy(&obj);
    ASSERT_TRUE(spy);
    spy->state = true;
    EXPECT_TRUE(obj.state);
}

TEST(spying, can_detect_spied_object_destruction)
{
    spy_pointer<SpiedObject> spy;
    ASSERT_FALSE(spy);
    {
        SpiedObject obj;
        EXPECT_FALSE(obj.is_spied());
        
        spy.reset(&obj);
        ASSERT_TRUE(spy);
        EXPECT_TRUE(obj.is_spied());
    }
    ASSERT_FALSE(spy);
}

TEST(spying, can_use_multiple_pointers)
{
    spy_pointer<SpiedObject> spy1, spy2;
    {
        SpiedObject obj;
        spy1 = &obj;
        ASSERT_TRUE(spy1);
        spy2 = &obj;
        ASSERT_TRUE(spy2);
    }
    ASSERT_FALSE(spy1);
    ASSERT_FALSE(spy2);
}

TEST(spying, can_construct_a_copy)
{
    SpiedObject obj;
    spy_pointer<SpiedObject> spy1(&obj);
    spy_pointer<SpiedObject> spy2(spy1);
    EXPECT_TRUE(spy2);
    spy1.reset();
    EXPECT_TRUE(spy2);
    EXPECT_TRUE(obj.is_spied());
}

TEST(spying, can_construct_by_move)
{
    SpiedObject obj;
    spy_pointer<SpiedObject> spy1(&obj);
    spy_pointer<SpiedObject> spy2(std::move(spy1));
    EXPECT_FALSE(spy1);
    EXPECT_TRUE(spy2);
}

TEST(spying, can_copy)
{
    SpiedObject obj1, obj2;
    spy_pointer<SpiedObject> spy1(&obj1), spy2(&obj2);
    ASSERT_TRUE(obj1.is_spied());
    ASSERT_TRUE(obj2.is_spied());
    spy1 = spy2;
    EXPECT_FALSE(obj1.is_spied());
    EXPECT_EQ(&obj2, &*spy1);
    EXPECT_TRUE(obj2.is_spied());
    EXPECT_TRUE(spy2);
}

TEST(spying, can_move)
{
    SpiedObject obj1, obj2;
    spy_pointer<SpiedObject> spy1(&obj1), spy2(&obj2);
    ASSERT_TRUE(obj1.is_spied());
    ASSERT_TRUE(obj2.is_spied());
    spy1 = std::move(spy2);
    EXPECT_FALSE(obj1.is_spied());
    EXPECT_EQ(&obj2, &*spy1);
    EXPECT_TRUE(obj2.is_spied());
    EXPECT_FALSE(spy2);
}

TEST(spying, can_notify_about_spying_satate_change)
{
    SpiedObject obj;
    EXPECT_FALSE(obj.spied_state_in_callback);
    spy_pointer<SpiedObject> spy(&obj);
    EXPECT_TRUE(obj.spied_state_in_callback);
    spy.reset();
    EXPECT_FALSE(obj.spied_state_in_callback);
}

TEST(spying, the_onli_first_and_last_spy_changes_spying_state)
{
    SpiedObject obj;
    spy_pointer<SpiedObject> spy1(&obj);

    obj.spied_state_in_callback = false;
    spy_pointer<SpiedObject> spy2(&obj);
    EXPECT_FALSE(obj.spied_state_in_callback);

    obj.spied_state_in_callback = true;
    spy2.reset();
    EXPECT_TRUE(obj.spied_state_in_callback);
}

TEST(spying, can_construct_copy_of_casted_type)
{
    ChildObject obj;
    spy_pointer<SpiedObject> spy1(&obj);
    spy_pointer<ChildObject> spy2(spy1);
    spy2->state = true;
    EXPECT_TRUE(obj.state);
}

TEST(spying, can_construct_by_move_of_casted_type)
{
    ChildObject obj;
    spy_pointer<SpiedObject> spy1(&obj);
    spy_pointer<ChildObject> spy2(std::move(spy1));
    spy2->state = true;
    EXPECT_TRUE(obj.state);
}

TEST(spying, can_copy_of_casted_type)
{
    ChildObject obj;
    spy_pointer<SpiedObject> spy1(&obj);
    spy_pointer<ChildObject> spy2;
    spy2 = spy1;
    spy2->state = true;
    EXPECT_TRUE(obj.state);
}

TEST(spying, can_move_casted_type)
{
    ChildObject obj;
    spy_pointer<SpiedObject> spy1(&obj);
    spy_pointer<ChildObject> spy2;
    spy2 = std::move(spy1);
    spy2->state = true;
    EXPECT_TRUE(obj.state);
}

TEST(spying, can_cast_type)
{
    ChildObject obj;
    spy_pointer<SpiedObject> spy(&obj);
    static_cast<spy_pointer<ChildObject>>(spy)->state = true;
    EXPECT_TRUE(obj.state);
}

TEST(spying, spied_object_construction_do_not_copy_spys)
{
    SpiedObject obj1;
    spy_pointer<SpiedObject> spy(&obj1);
    SpiedObject obj2(obj1);
    EXPECT_FALSE(obj2.is_spied());
    EXPECT_TRUE(obj1.is_spied());
    EXPECT_TRUE(spy);
}

TEST(spying, spied_object_construction_do_not_move_spys)
{
    SpiedObject obj1;
    spy_pointer<SpiedObject> spy(&obj1);
    SpiedObject obj2(std::move(obj1));
    EXPECT_FALSE(obj2.is_spied());
    EXPECT_FALSE(obj1.is_spied());
    EXPECT_FALSE(spy);
}

TEST(spying, spied_object_copying_do_not_copy_spys)
{
    SpiedObject obj1;
    spy_pointer<SpiedObject> spy(&obj1);
    SpiedObject obj2;
    obj2 = obj1;
    EXPECT_FALSE(obj2.is_spied());
    EXPECT_TRUE(obj1.is_spied());
    EXPECT_TRUE(spy);
}

TEST(spying, spied_object_moving_do_not_move_spys)
{
    SpiedObject obj1;
    spy_pointer<SpiedObject> spy(&obj1);
    SpiedObject obj2;
    obj2 = std::move(obj1);
    EXPECT_FALSE(obj2.is_spied());
    EXPECT_FALSE(obj1.is_spied());
    EXPECT_FALSE(spy);
}

