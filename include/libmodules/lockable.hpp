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

#include <functional>

namespace mtl
{
    struct no_lockable
    {
        void lock() {}
        void unlock() {}
    };

    struct I_locable
    {
        virtual void lock() = 0;
        virtual void unlock() = 0;
    };

    struct interface_locker
    {
        interface_locker(I_locable& locker) : _locker(locker) {}
        void lock() { _locker.get().lock(); }
        void unlock() { _locker.get().unlock(); }
    private:
        std::reference_wrapper<I_locable> _locker;
    };
}
