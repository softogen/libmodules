/* -*- mode: c++; c-file-style: "bsd"; c++-basic-offset: 4; indent-tabs-mode nil -*- */
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
#pragma once

#include "linked_list.hpp"

#include <type_traits>
#include <utility>

namespace mtl
{
    // Spy pointer is a kind of auto pointer that is close to std::weak_ptr. It does not take
    // ownership on referenced object but resets itself when object is destroyed.
    //
    // It could point object placed in any kind of memory: global scope, stack or heap. It also
    // works fine with objects stored as local members of other objects.
    //
    // There is not magic here. Pointed object have to store a linked list of all spy pointers that
    // are reference it. At the destruction moment the object release all pointers in the list.
    // This architecture defines the main limit of this pointer: it could not be used in
    // multi-threading environment. The only single thread could owner the linked list of spy
    // pointers.
    //
    // The pointer object should inherit enable_spying class to handle pointer's linked list and
    // implement required behavior. The spy pointer template should be specialized by pointer
    // object type.

    template<typename object_type, typename object_base = object_type>
    class spy_pointer;

    // Template specialization is used to resolve ambiguous calls in case of multiple inheritance.
    template<typename object_type>
    class enable_spying
    {
    public:
        bool empty() noexcept { return !_list_head; }

        // Release all pointers that reference this object and stored in the linked list
        void clear() noexcept;

    protected:
        // Constructors, destructor and copy operators has nothing specific. They take no ownership
        // of linked list from other object, but clears other object in case if it was moved. It
        // means that spy pointer operates like regular pointer and do not follow to object if it
        // was copied or moved.
        enable_spying() noexcept
        {
            // Types should be defined to call `std::is_base_of`.
            static_assert(std::is_base_of<enable_spying, object_type>::value, "The enable_spying template should be specified by derived class type.");
        };

        ~enable_spying() noexcept { clear(); }
        explicit enable_spying(const enable_spying<object_type>&other) noexcept {}
        explicit enable_spying(enable_spying<object_type> && other) noexcept { other.clear(); }

        enable_spying<object_type>& operator =(const enable_spying<object_type>&other) noexcept { return *this; }
        enable_spying<object_type>& operator =(enable_spying<object_type> && other) noexcept { other.clear(); return *this; }

        // Child type could by notified when it has new spy or lose the last one. It could overload
        // this signal to perform some specific actions.
        //
        // Note: this function is executed in destructor of friend class. So, it should not throw.
        virtual void on_spying_state_changed() noexcept {}

    private:
        // Pointers should be able to add itself in the linked list
        template<typename PointerType, typename object_base>
        friend class spy_pointer;

        // List head could be modified in constant object
        mutable spy_pointer<object_type>* _list_head = nullptr;
    }; // class enable_spying

    template<typename object_type, typename object_base>
    class spy_pointer final
        : private enable_linking_in_list<spy_pointer<object_base>>
    {
        static_assert(std::is_base_of<object_base, object_type>::value, "The spy pointer should be specified by derived and base classes.");
    public:
        using base_type = enable_linking_in_list<spy_pointer<object_base>>;
        using type = spy_pointer<object_type, object_base>;

        spy_pointer(object_type* p_object = nullptr) noexcept
            : _p_object(p_object)
        {
            if (!_p_object)
                return;

            enable_spying<object_base>* tmp = _p_object;
            // Place the pointer at the head of the linked list
            this->insert(tmp->_list_head);
            // If it is the first spy we should notify the object
            if(!this->next())
                tmp->on_spying_state_changed();
        }

        ~spy_pointer() noexcept
        {
            if (!_p_object)
                return;

            // Exclude itself from the list 
            this->unlink();
            // There should no any spy pointed to the object while we notifying the object
            // Yes, this object also should be released
            // So, we clen member pointer before we call notification callback
            enable_spying<object_base>* tmp = _p_object;
            _p_object = nullptr;
            if (!tmp->_list_head)
                tmp->on_spying_state_changed();
        }

        template<typename other_type>
        type& swap(spy_pointer<other_type, object_base>& other) noexcept
        {
            // Swap linked lists
            base_type::swap(other);
            // Then swap pointers
            enable_spying<object_base>* tmp = _p_object;
            _p_object = static_cast<object_type*>(other._p_object);
            other._p_object = static_cast<other_type*>(tmp);

            return *this;
        }

        // The rest of copy constructors, copy operators and other operations
        explicit spy_pointer(const type&  other) noexcept : spy_pointer(other._p_object) {}
        template<typename other_type>
        explicit spy_pointer(const spy_pointer<other_type, object_base>&  other) noexcept : spy_pointer(static_cast<object_type*>(other._p_object)) {}

        explicit spy_pointer(type&& other) noexcept                                   {                                    swap(other); }
        template<typename other_type>
        explicit spy_pointer(spy_pointer<other_type, object_base>&& other) noexcept   {                                    swap(other); }

        type& operator =(const type&  other) noexcept                                 { type tmp(other);            return swap(tmp); }
        type& operator =(      type&& other) noexcept                                 { type tmp(std::move(other)); return swap(tmp); }
        template<typename other_type>
        type& operator =(const spy_pointer<other_type, object_base>&  other) noexcept { type tmp(other);            return swap(tmp); }
        template<typename other_type>
        type& operator =(      spy_pointer<other_type, object_base>&& other) noexcept { type tmp(std::move(other)); return swap(tmp); }

        template<typename other_type>
        operator spy_pointer<other_type, object_base>() const noexcept { return spy_pointer<other_type, object_base>(*this); }

        type& reset(     object_type* p_object = nullptr) noexcept { type tmp(p_object); return swap(tmp); }
        type& operator =(object_type* p_object)           noexcept { return reset(p_object); }
        
        operator bool()           const noexcept { return !!_p_object; }
        object_type* operator->() const noexcept { return   _p_object; }
        object_type& operator *() const noexcept { return  *_p_object; }
        object_type* get()        const noexcept { return   _p_object; }
        operator   object_type*() const noexcept { return   _p_object; }

    private:
        // Pointers could point to different type casts of one object
        template<typename other_type, typename OtherBase>
        friend class spy_pointer;
        // Object should be able to release the pointer and exclude it from the list
        friend base_type;

        object_type* _p_object = nullptr;
    }; // class spy_pointer

    // The clear method has to be defined after the spy_pointer class definition to be able access
    // it's members.
    template<typename object_type>
    inline void enable_spying<object_type>::clear() noexcept
    {
        while(_list_head)
            _list_head->reset();
    }

    template<typename left_type, typename right_type, typename object_base>
    bool operator ==(const spy_pointer<left_type, object_base>& left, const spy_pointer<right_type, object_base>& right) noexcept
    {
        return static_cast<enable_spying<object_base>*>(left.get()) == static_cast<enable_spying<object_base>*>(right.get());
    }

    template<typename first_type, typename second_type, typename object_base>
	void swap(spy_pointer<first_type, object_base>& first, spy_pointer<second_type, object_base>& second) noexcept
    {
        first.swap(second);
    }
}
