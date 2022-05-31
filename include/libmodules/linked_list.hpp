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
#pragma once

#include <utility>

namespace mtl
{
    // Linked list is a technique that allows to linked multiple objects in order one by one.  It
    // isn't a container for objects like vector or list. It' doesn't handle lifetime of linked
    // objects. It contains nothing and just setup rules how objects points to each other.
    //
    //   The main idea is to organize objects in bidirectional list where each object has pointer
    // to the next and previous one in the row. This construction has one problem place at the head
    // of the list. Each list method should cover some exception case and fix pointer to the head.
    //
    //   I didn't meet any case in my practice when I required backward iterations through the
    // list. So, I decided to use back pointer in a different way to simplify the algorithms. Each
    // element points back to the simple pointer to this object but not whole previous object.
    //
    //          ---------    ---------
    // _next -> | _next-+--->| _next | ...
    //   ^      |   ^   |    |       |
    //   |      |   ----+--  |       |
    //   |      |       | |  |       |
    //   -------+-_prev | ---+-_prev |
    //          ---------    ---------
    //
    // This data structure has no difference between previous item and pointer to the list head.
    // Each item is the had of the sublist. It simplifies list method dramatically.
    //
    //   Objects in the linked list are managed outside of the list. It means, that they could be
    // placed everywhere in the memory. They could be in the heap or at the program stack. Moreover
    // they could be a member of other objects. Linked list is just connect them to each other.
    // This fact allows to link different type objects. The only base virtual interface could be
    // linked into the list.

    // The linked object have to inherit from the linked list class to enable required
    // functionality.  The linked list takes object type as a template parameter to hide static
    // cast and increase usability.
    template<typename T>
    class enable_linking_in_list
    {
    public:
        using type = enable_linking_in_list<T>;
        using next_pointer = T*;
        using prev_pointer = next_pointer*;
        using reference    = type&;
        using rvalue       = type&&;

        enable_linking_in_list() = default;

        // Construct new object and insert it into the head of the provided list.
        // This constructor could be used to stack objects into the list one by one.
        enable_linking_in_list(next_pointer& list)
        {
            // This assert is placed here to delay check until parent object is created.
            static_assert(std::is_base_of<enable_linking_in_list<T>, T>::value,
                          "enable_linking_in_list should be base of template parameter type");
            // Place self into the list head
            _prev = &list;
            _next = list;
            // Fix list structure
            if (list)
                list->_prev = &_next;
            list = static_cast<next_pointer>(this);
        }

        reference swap(reference other)
        {
            // Swap content
            // It isn't a simple operation due to ceases when neighborhoods are swapped
            prev_pointer prev_tmp = _prev;
            next_pointer next_tmp = _next;
            next_pointer this_tmp = static_cast<next_pointer>(this);
            next_pointer other_tmp = static_cast<next_pointer>(&other);
            _prev =       (other._prev == &_next)       ? &other._next : other._prev;
            _next =       (other._next == this_tmp)     ? other_tmp    : other._next;
            other._prev = (prev_tmp    == &other._next) ? &_next       : prev_tmp;
            other._next = (next_tmp    == other_tmp)    ? this_tmp     : next_tmp;
            // Fix heads
            if (_prev)
                *_prev = this_tmp;
            if (other._prev)
                *other._prev = other_tmp;
            // Fix tails
            if (_next)
                _next->_prev = &_next;
            if (other._next)
                other._next->_prev = &other._next;
            return *this;
        }

        // The rest of copy constructors, copy operators and other operations
        // Copy constructor inserts the new item after the original item.
        // This allows to queue item in the list.
        enable_linking_in_list (const reference     prev)  : enable_linking_in_list(prev._next) {}
        enable_linking_in_list (      rvalue        other) { swap(other); }
        reference operator=    (const reference     prev)  { return insert_after(prev); }
        reference operator=    (      rvalue        other) { type tmp(std::move(other)); return swap(tmp); }
        reference insert       (      next_pointer& list)  { type tmp(list);             return swap(tmp); }
        reference unlink()                                 { type tmp;                   return swap(tmp); }
        reference insert_after (const reference     prev)  { type tmp(prev);             return swap(tmp); }
        reference insert_before(const reference     next)  { insert_after(next);         return swap(next); }

        // These getters allow to iterate down by linked list.
        // Constant object do not removes constant access.
        const next_pointer next() const { return _next; }
              next_pointer next()       { return _next; }

        // Cast to bool is true in case if the item is linked into the list.
        // It means that it has not empty pointer to previous item.
        operator bool() const { return !!_prev; }

    protected:
        ~enable_linking_in_list()
        {
            // Exclude self from the list
            if (_prev)
                *_prev = _next;
            if (_next)
                _next->_prev = _prev;
        }

    private:
        prev_pointer _prev = nullptr;
        next_pointer _next = nullptr;
    }; // class enable_linking_in_list
} // namespace mtl
