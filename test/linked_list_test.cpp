/******************************************************************************

Project libmodule
Copyright 2017 Sergey Ushakov

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
******************************************************************************/

#include <libmodules/linked_list.hpp>
#include <gmock/gmock.h>

using namespace modules;

struct object
  : public enable_linking_in_list<object>
{
  object(object** prev = nullptr)
    : enable_linking_in_list<object>(prev)
  {}
};

TEST(linking_in_list, can_construct_with_head_pointer)
{
  object* ptr = nullptr;
  object head(&ptr);
  EXPECT_EQ(&head, ptr);
  EXPECT_TRUE(head);
  EXPECT_EQ(nullptr, head.next());
} 

TEST(linking_in_list, can_construct_before_head)
{
  object* ptr = nullptr;
  object old_head(&ptr);
  object new_head(old_head);
  EXPECT_EQ(&new_head, ptr);
  EXPECT_TRUE(new_head);
  EXPECT_EQ(&old_head, new_head.next());
  EXPECT_TRUE(old_head);
}

TEST(linking_in_list, destruction_extracts)
{
  object* ptr = nullptr;
  {
    object tail(&ptr);
    EXPECT_EQ(&tail, ptr);
    EXPECT_TRUE(tail);
  }
  EXPECT_EQ(nullptr, ptr);
}

TEST(linking_in_list, can_desctruct_head)
{
  object* ptr = nullptr;
  object tail(&ptr);
  EXPECT_EQ(&tail, ptr);
  {
    object head(tail);
    EXPECT_EQ(&head, ptr);
    EXPECT_EQ(&tail, head.next());
    EXPECT_TRUE(head);
  }
  EXPECT_EQ(&tail, ptr);
}

TEST(linking_in_list, can_swap_items)
{
  object* ptr1 = nullptr;
  object tail1(&ptr1);
  object head1(tail1);

  object* ptr2 = nullptr;
  object tail2(&ptr2);
  object head2(tail2);

  head1.swap(head2);

  EXPECT_EQ(&head2, ptr1);
  EXPECT_EQ(&tail1, head2.next());
  EXPECT_EQ(&head1, ptr2);
  EXPECT_EQ(&tail2, head1.next());
}

TEST(linking_in_list, can_move_construct)
{
  object* ptr = nullptr;
  object tail(&ptr);
  object old_head(tail);

  object new_head(std::move(old_head));

  EXPECT_EQ(&new_head, ptr);
  EXPECT_EQ(&tail, new_head.next());
  EXPECT_TRUE(new_head);
  EXPECT_TRUE(tail);

  EXPECT_EQ(nullptr, old_head.next());
  EXPECT_FALSE(old_head);
}

TEST(linking_in_list, can_copy_as_insert_before)
{
  object* ptr1 = nullptr;
  object tail1(&ptr1);

  object* ptr2 = nullptr;
  object tail2(&ptr2);
  object head(tail2);
  
  head = tail1;

  EXPECT_EQ(&head, ptr1);
  EXPECT_EQ(&tail1, head.next());
  EXPECT_TRUE(head);

  EXPECT_EQ(&tail2, ptr2);
  EXPECT_TRUE(tail2);
}

TEST(linking_in_list, can_move)
{
  object* ptr1 = nullptr;
  object tail1(&ptr1);
  object old_head(tail1);

  object* ptr2 = nullptr;
  object tail2(&ptr2);
  object new_head(tail2);

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
  object tail(&ptr);
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
}

TEST(linking_in_list, can_insert_after)
{
  object tail;
  object head(tail);
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
  object tail(&ptr);
  object head(tail);

  head.unlink();

  EXPECT_EQ(&tail, ptr);
  EXPECT_EQ(nullptr, head.next());
  EXPECT_TRUE(tail);
  EXPECT_FALSE(head);
}
