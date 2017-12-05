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

#include "lockable.hpp"

namespace modules
{
  template< typename T> class spy_pointer;
  template< typename T> class enable_spying
  {
  public:
    ~enable_spying() { _p_spy->reset(); }
    void add_spy(spy_pointer<T>& spy) { _p_spy = &spy; }
  private:
    spy_pointer<T>* _p_spy = nullptr;
  };
  
  template< typename T> class spy_pointer
  {
  public:
    spy_pointer(T* p_object = nullptr) : _p_object(p_object) { if(_p_object) _p_object->add_spy(*this); }
    T* operator ->() { return _p_object; }
    operator bool() const { return !!_p_object; }
    void reset(T* p_object = nullptr) { *this = spy_pointer<T>(p_object); if(_p_object) _p_object->add_spy(*this); }
  private:
    T* _p_object;
  };
}
