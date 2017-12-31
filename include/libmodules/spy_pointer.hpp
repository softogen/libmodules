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

#include "linked_list.hpp"
#include <utility>

namespace modules
{
  template<typename T>
  class spy_pointer;

  class enable_spying
  {
  public:
    bool is_spied() { return !!_p_spy; }

  protected:
    enable_spying() = default;
    enable_spying(const enable_spying& other) {}
    enable_spying(enable_spying&& other) { other.clear(); }
    virtual ~enable_spying() { clear(); }

    enable_spying& operator =(const enable_spying& other) { return *this; }
    enable_spying& operator =(enable_spying&& other) { other.clear(); return * this; }

    virtual void on_spying_state_changed() {}

  private:
    template<typename PointerType>
    friend class spy_pointer;

    void clear();
    mutable spy_pointer<enable_spying>* _p_spy = nullptr;
  };

  template<typename T>
  class spy_pointer
    : private enable_linking_in_list<spy_pointer<enable_spying>>
  {
  public:
    using Type = spy_pointer<T>;
    using BaseType = enable_linking_in_list<spy_pointer<enable_spying>>;

    spy_pointer(T* p_object = nullptr)
      : BaseType(p_object ? &p_object->_p_spy : nullptr)
      , _p_object(p_object)
    {
      if (_p_object && !next())
	_p_object->on_spying_state_changed();
    }

    ~spy_pointer()
    {
      if (!_p_object)
	return;

      unlink();
      T* tmp_ptr = _p_object;
      _p_object = nullptr;
      if (!tmp_ptr->_p_spy)
        tmp_ptr->on_spying_state_changed();
    }

    spy_pointer(const Type& other) : spy_pointer(other._p_object) {}
    spy_pointer(Type&& other) { swap(other); }
    Type& operator =(const Type& other) { Type tmp(other); return swap(tmp); }
    Type& operator =(Type&& other) { Type tmp(std::move(other)); return swap(tmp); }

    template<typename OtherType>
    Type& swap(spy_pointer<OtherType>& other)
    {
      BaseType::swap(other);
      T* tmp = _p_object;
      _p_object = static_cast<T*>(other._p_object);
      other._p_object = static_cast<OtherType*>(tmp);
      return *this;
    }

    template<typename OtherType>
    spy_pointer(const spy_pointer<OtherType>& other)
      : spy_pointer(static_cast<T*>(other._p_object))
    {}

    template<typename OtherType>
    spy_pointer(spy_pointer<OtherType>&& other) { swap(other); }

    template<typename OtherType>
    Type& operator =(const spy_pointer<OtherType>& other) { Type tmp(other); return swap(tmp); }

    template<typename OtherType>
    Type& operator =(spy_pointer<OtherType>&& other) { Type tmp(std::move(other)); return swap(tmp); }

    //template<typename OtherType>
    //operator spy_pointer<OtherType>() { return spy_pointer<OtherType>(*this); }

    Type& reset(T* p_object = nullptr) { Type tmp(p_object); return swap(tmp); }
    Type& operator =(T* p_object) { return reset(p_object); }
    operator bool() const { return !!_p_object; }
    T* operator ->() const { return _p_object; }
    T& operator *() const { return *_p_object; }
    T* get() const { return _p_object; }

    template<typename OtherType>
    bool operator ==(spy_pointer<OtherType>& other)
    {
      return static_cast<enable_spying*>(_p_object) == static_cast<enable_spying*>(other._p_object);
    }

  private:
    template<typename OtherType>
    friend class spy_pointer;
    friend class enable_linking_in_list;
    T* _p_object = nullptr;
  };

  inline void enable_spying::clear()
  {
    while(_p_spy)
      _p_spy->reset();
  }
}
