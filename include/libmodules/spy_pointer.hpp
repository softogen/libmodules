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

#include "linked_list.hpp"
#include <utility>

namespace mtl
{
    // Spy pointer is a kind of autopointer that is close to std::weak_ptr. It does not take
    // ownership on referenced object but resets itself when object is destroyed.
    //
    // It could point object placed in any kinde of memory: global scope, stack or heap. It also
    // works fine with objects stored as local members of other objects.
    //
    // Threre is not magic here. Pointed object have to store a linked list of all spy pointers that
    // are reference it. At the destruction moment the object release all pointers in the list. Ths
    // architecture defines the main limit of this pointer: it could not be used in multythreading
    // environment. The only single thred could owner the linked list of spy pointers.
    //
    // The pointer object should inherit enable_spying class to handle pointer's linked list and
    // implement required behaviour. The spy pointer template should be specialized by pointer
    // object type.

    template<typename T>
    class spy_pointer;

    class enable_spying
    {
        // Pointers should be able to add itself in the linked list
        template<typename PointerType>
        friend class spy_pointer;

        // List head could be modified in constant object
        mutable spy_pointer<enable_spying>* _list_head = nullptr;

    protected:
        // Constructors, destrutor and copy operators has nothing specific. They take no ownership
        // of linked list from other object, but clears other object in case if it was moved. It
        // means that spy pointer operates like regular pointer and do not follow to object if it
        // was copyed or moved.
        enable_spying() = default;
        enable_spying(const enable_spying&  other) {}
        enable_spying(      enable_spying&& other) { other.clear(); }
        virtual ~enable_spying() { clear(); }

        enable_spying& operator =(const enable_spying&  other) {                return *this; }
        enable_spying& operator =(      enable_spying&& other) { other.clear(); return *this; }

        // Child type could by notified when it has new spy or lose the last one. It could overload
        // this signal to perform some specific atcions.
        virtual void on_spying_state_changed() {}

    public:
        bool empty() { return !_list_head; }

        // Release all pointers that reference this object and stored in the linked list
        void clear();
    };

    template<typename T>
    class spy_pointer
        : private enable_linking_in_list<spy_pointer<enable_spying>>
    {
        using base_type = enable_linking_in_list<spy_pointer<enable_spying>>;

        // Pointers could point to different type casts of one object
        template<typename other_type>
        friend class spy_pointer;
        // Object should be able to release the pointer and exclue it from the list
        friend class base_type;

        T* _p_object = nullptr;

    public:
        using type = spy_pointer<T>;

        spy_pointer(T* p_object = nullptr)
            : _p_object(p_object)
        {
            if (!_p_object)
                return;

            // Place the pointer at the head of the linked list
            insert(_p_object->_list_head);
            // If it is the first spy we should notify the object
            if(!next())
                _p_object->on_spying_state_changed();
        }

        ~spy_pointer()
        {
            if (!_p_object)
                return;

            // Exclude itself from the list 
            unlink();
            // There should no any spy pointed to the object while we notyfying the object
            // Yes, this object also should be released
            // So, we clen member pointer before we call notification callback
            T* tmp_ptr = _p_object;
            _p_object = nullptr;
            if (!tmp_ptr->_list_head)
                tmp_ptr->on_spying_state_changed();
        }

        template<typename other_type>
        type& swap(spy_pointer<other_type>& other)
        {
            // Swap linked lists
            base_type::swap(other);
            // THen swap pointers
            T* tmp = _p_object;
            _p_object = static_cast<T*>(other._p_object);
            other._p_object = static_cast<other_type*>(tmp);

            return *this;
        }

        // The rest of copy constructors, copy operators and other operations
        spy_pointer(     const type&  other) :                    spy_pointer(other._p_object) {}
        template<typename other_type>
        spy_pointer(const      spy_pointer<other_type>&  other) : spy_pointer(static_cast<T*>(other._p_object)) {}

        spy_pointer(           type&& other) {                                                       swap(other); }
        template<typename other_type>
        spy_pointer(           spy_pointer<other_type>&& other) {                                    swap(other); }

        type& operator =(const type&  other) { type tmp(other);                               return swap(tmp); }
        type& operator =(      type&& other) { type tmp(std::move(other));                    return swap(tmp); }
        template<typename other_type>
        type& operator =(const spy_pointer<other_type>&  other) { type tmp(other);            return swap(tmp); }
        template<typename other_type>
        type& operator =(      spy_pointer<other_type>&& other) { type tmp(std::move(other)); return swap(tmp); }

        template<typename other_type>
        operator spy_pointer<other_type>() const { return spy_pointer<other_type>(*this); }

        type& reset(     T* p_object = nullptr) { type tmp(p_object); return swap(tmp); }
        type& operator =(T* p_object)           { return reset(p_object); }
        
        operator bool() const { return !!_p_object; }
        T* operator->() const { return   _p_object; }
        T& operator *() const { return  *_p_object; }
        T* get()        const { return   _p_object; }
        operator   T*() const { return   _p_object; }

        template<typename other_type>
        bool operator ==(spy_pointer<other_type>& other)
        {
            return static_cast<enable_spying*>(_p_object) == static_cast<enable_spying*>(other._p_object);
        }
    };

    // The clear method has to be defined after the spy_pointer class definition to be able access it's members.
    inline void enable_spying::clear()
    {
        while(_list_head)
            _list_head->reset();
    }
}
