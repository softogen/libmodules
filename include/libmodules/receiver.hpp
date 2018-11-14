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

#include "signal.hpp"

#include <vector>
#include <algorithm>

namespace mtl
{
    // This file is to implement signal transmission feature. Please see signal.hpp for details.
    // This file contains Receiver templates implementation that process signals received from
    // corresponded transmitter.

    // Simple receiver implementation. It just implement signal table and perform signal
    // immediately.
    template<typename signal_table>
    class receiver
        : public signal_table
        , public transmitter<signal_table>
    {
    public:
        receiver() : transmitter<signal_table>(this) {}
        receiver(const receiver& other) : receiver() {}
        receiver(const receiver&& other) : receiver() {}

        receiver& operator=(const receiver& other) { return *this; }
        receiver& operator=(const receiver&& other) { return *this; }
    };
} // namespace mtl
