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

#include "spy_pointer.hpp"

#include <functional>

namespace mtl
{
    // This file is to implement signal transmission feature. There is multiple core objects in the
    // library:
    // Signal - some occasion for concerted action which can be accompanied by some information
    // Signal table - group of signals represented as one virtual interface
    // Emitter - object that can emit signals from specified signal table
    // Transmitter - object that can be attach to emitter and can catch it's signals
    // Receiver - Implementation of reaction to the signals were catch by transmitter
    //
    // General use case of the library is to:
    // - Declare signal table as virtual interface with empty void functions
    // - Inherit YourClass from emitter<signal_table>
    // - Send signals like send(&signal_table::signal_name, args...);
    // - Inherit YourListenerClasses from receiver<signal_table> and implement signal reactions as
    //   override functions
    // - Do YourClass.attach(YourListenerClasses)
    //
    // More complicated case could use packed signals - callable object, that could be stored,
    // transfered and processed in multiple ways without detailed signal specification. For example
    // receiver could store packed signals in the queue to process them in the other thread.
    //
    // You can attach multiple receivers to one emitter or one receiver to multiple emitters.
    // Multiple inheritance could be used to receive signals from emitters of different types.
    // Method send could return false in case if emitter instance is destroyed while sending.

    template<typename signal_table>
    using packed_signal = std::function<void(signal_table&)>;

    template<typename signal_table>
    class transmitter
        : private enable_spying<transmitter<signal_table>>
    {
        using base = enable_spying<transmitter<signal_table>>;
        signal_table* _receiver = nullptr;
        friend class spy_pointer<transmitter<signal_table>>; // Provide access to parent

    public:
        transmitter(signal_table* receiver = nullptr) : _receiver(receiver) {}

        // Returns receiver's interface
        // If no receiver specified then packed signal will be transmitted
        signal_table* get() { return _receiver; }

        // Detach from all emitters
        void reset() { base::clear(); }
        // Check if it is attached to any emitter
        bool empty() { return base::empty(); }

        // Unpack packed signal and transmit it to the receiver
        // This method could be overridden to specify transmission
        virtual void transmit_signal(const packed_signal<signal_table>& call)
        {
            if (!_receiver)
                return;

            call(*_receiver);
        }
    };
} // namespace mtl
