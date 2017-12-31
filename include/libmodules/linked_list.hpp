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

#include <utility>

namespace modules
{
  template<typename T>
  class enable_linking_in_list
  {
    mutable T** _prev = nullptr;
    mutable T* _next = nullptr;
  public:
    enable_linking_in_list(T** prev = nullptr)
    {
      static_assert(std::is_base_of<enable_linking_in_list<T>, T>::value, "enable_linking_in_list should be base of template parameter T");
      if (!prev)
	return;
      _next = *prev;
      _prev = prev;
      if (*prev)
	(**prev)._prev = &_next;
      *prev = static_cast<T*>(this);
    }

    ~enable_linking_in_list()
    {
      if (_prev)
	*_prev = _next;
      if (_next)
	_next->_prev = _prev;
    }

    enable_linking_in_list& swap(enable_linking_in_list& other)
    {
      if (_prev)
	*_prev = static_cast<T*>(&other);
      if (other._prev)
	*other._prev = static_cast<T*>(this);
      if (_next)
	_next->_prev = &other._next;
      if (other._next)
	other._next->_prev = &_next;
      std::swap(_prev, other._prev);
      std::swap(_next, other._next);
      return *this;
    }

    enable_linking_in_list(const enable_linking_in_list& next) : enable_linking_in_list(next._prev)
    {
      if (_next == &next)
	return;
      _next = const_cast<T*>(static_cast<const T*>(&next));
      _next->_prev = &_next;
    }
    
    enable_linking_in_list(enable_linking_in_list&& other) { swap(other); }
    enable_linking_in_list& operator =(   const enable_linking_in_list&  next)  { return insert_before(next); }
    enable_linking_in_list& operator =(         enable_linking_in_list&& other) { enable_linking_in_list tmp(std::move(other)); return swap(tmp); }
    enable_linking_in_list& insert_before(const enable_linking_in_list&  next)  { enable_linking_in_list tmp(next);             return swap(tmp); }
    enable_linking_in_list& insert_after( const enable_linking_in_list&  prev)  { enable_linking_in_list tmp(&prev._next);      return swap(tmp); }
    enable_linking_in_list& unlink()                                            { enable_linking_in_list tmp;                   return swap(tmp); }

    const T* next() const { return _next; }
    T* next() { return _next; }
    operator bool() const { return !!_prev; }
  };
}
