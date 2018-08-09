/***************************************************************************************************

Project libmodule
Copyright 2017 Sergey Ushakov

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software distributed under the License is
distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
implied.  See the License for the specific language governing permissions and limitations under the
License.
***************************************************************************************************/

#include <libmodules/linked_list.hpp>
#include <gmock/gmock.h>

using namespace mtl;

struct object
    : public enable_linking_in_list<object>
{
    object() = default;
    object(object*& next)
        : enable_linking_in_list<object>(next)
    {}
};

TEST(linking_in_list, can_construct_with_head_pointer)
{
    object* ptr = nullptr;
    object head(ptr);
    EXPECT_EQ(&head, ptr);
    EXPECT_TRUE(head);
    EXPECT_EQ(nullptr, head.next());
} 

TEST(linking_in_list, can_stack_by_construction)
{
    object* ptr = nullptr;
    object item(ptr);
    object head(ptr);
    EXPECT_EQ(&head, ptr);
    EXPECT_TRUE(item);
    EXPECT_EQ(&item, head.next());
} 

TEST(linking_in_list, can_queue_by_construction)
{
    object* ptr = nullptr;
    object head(ptr);
    object item(head);
    EXPECT_EQ(&head, ptr);
    EXPECT_EQ(&item, head.next());
    EXPECT_TRUE(item);
}

TEST(linking_in_list, destruction_of_object_restores_the_list)
{
    object* ptr = nullptr;
    {
        object item(ptr);
        EXPECT_EQ(&item, ptr);
    }
    EXPECT_EQ(nullptr, ptr);
}

TEST(linking_in_list, can_desctruct_head)
{
    object* ptr = nullptr;
    object tail(ptr);
    {
        object head(ptr);
        EXPECT_EQ(&head, ptr);
    }
    EXPECT_EQ(&tail, ptr);
}

TEST(linking_in_list, can_stack_into_list)
{
    object* ptr = nullptr;
    object tail(ptr);
    object item;

    item.insert(ptr);
    EXPECT_EQ(&item, ptr);
    EXPECT_EQ(&tail, item.next());
    EXPECT_TRUE(item);
    EXPECT_TRUE(tail);
}

TEST(linking_in_list, can_swap_items)
{
    object* ptr1 = nullptr;
    object head1(ptr1);
    object tail1(head1);

    object* ptr2 = nullptr;
    object head2(ptr2);
    object tail2(head2);

    head1.swap(head2);

    EXPECT_EQ(&head2, ptr1);
    EXPECT_EQ(&tail1, head2.next());
    EXPECT_EQ(&head1, ptr2);
    EXPECT_EQ(&tail2, head1.next());
}

TEST(linking_in_list, can_swap_neighbourhoods_forward)
{
    object* ptr = nullptr;
    object second(ptr);
    {
        object first(ptr);

        first.swap(second);
        EXPECT_EQ(&second, ptr);
        EXPECT_EQ(&first, second.next());
        EXPECT_TRUE(first);
        EXPECT_TRUE(second);
        EXPECT_EQ(nullptr, first.next());
    }
    EXPECT_EQ(nullptr, second.next());
}

TEST(linking_in_list, can_swap_neighbourhoods_backward)
{
    object* ptr = nullptr;
    object second(ptr);
    {
        object first(ptr);

        second.swap(first);
        EXPECT_EQ(&second, ptr);
        EXPECT_EQ(&first, second.next());
        EXPECT_TRUE(first);
        EXPECT_TRUE(second);
        EXPECT_EQ(nullptr, first.next());
    }
    EXPECT_EQ(nullptr, second.next());
}

TEST(linking_in_list, can_move_construct)
{
    object* ptr = nullptr;
    object old_head(ptr);
    object tail(old_head);

    object new_head(std::move(old_head));

    EXPECT_EQ(&new_head, ptr);
    EXPECT_EQ(&tail, new_head.next());
    EXPECT_TRUE(new_head);
    EXPECT_TRUE(tail);

    EXPECT_EQ(nullptr, old_head.next());
    EXPECT_FALSE(old_head);
}

TEST(linking_in_list, can_copy_as_insert_after)
{
    object* ptr1 = nullptr;
    object list1(ptr1);

    object* ptr2 = nullptr;
    object item(ptr2);
    object list2(item);
  
    item = list1;

    EXPECT_EQ(&item, list1.next());
    EXPECT_TRUE(item);

    EXPECT_EQ(&list2, ptr2);
    EXPECT_TRUE(list2);
}

TEST(linking_in_list, can_move)
{
    object* ptr1 = nullptr;
    object old_head(ptr1);
    object tail1(old_head);

    object* ptr2 = nullptr;
    object new_head(ptr2);
    object tail2(new_head);

    new_head = std::move(old_head);

    EXPECT_EQ(&new_head, ptr1);
    EXPECT_EQ(&tail1, new_head.next());
    EXPECT_TRUE(new_head);

    EXPECT_EQ(&tail2, ptr2);

    EXPECT_FALSE(old_head);
    EXPECT_EQ(nullptr, old_head.next());
}

TEST(linking_in_list, can_insert_before)
{
    object* ptr = nullptr;
    object tail(ptr);
    object head;

    head.insert_before(tail);

    EXPECT_EQ(&head, ptr);
    EXPECT_EQ(&tail, head.next());
    EXPECT_TRUE(head);
}

TEST(linking_in_list, can_insert_before_head)
{
    object tail;
    object new_head;

    new_head.insert_before(tail);

    EXPECT_EQ(&tail, new_head.next());
    EXPECT_TRUE(tail);
    EXPECT_FALSE(new_head);
}

TEST(linking_in_list, can_insert_after)
{
    object head;
    object tail(head);
    object item;

    item.insert_after(head);

    EXPECT_EQ(&item, head.next());
    EXPECT_EQ(&tail, item.next());
    EXPECT_TRUE(item);
}

TEST(linking_in_list, can_insert_after_tail)
{
    object head;
    object tail;

    tail.insert_after(head);

    EXPECT_EQ(&tail, head.next());
    EXPECT_TRUE(tail);
}

TEST(linking_in_list, can_remove_from_list)
{
    object* ptr = nullptr;
    object head(ptr);
    object tail(head);

    head.unlink();

    EXPECT_EQ(&tail, ptr);
    EXPECT_EQ(nullptr, head.next());
    EXPECT_TRUE(tail);
    EXPECT_FALSE(head);
}
