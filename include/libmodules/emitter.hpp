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

#include "spy_pointer.hpp"

#include <functional>
#include <vector>
#include <algorithm>

namespace mtl
{
    // This file is to implement signal transmission feature. There is multiple core objects in the
    // library:
    // Signal - some occasion for concerted action which can be accompanied by some information
    // Signal table - group of signals represented as one virtual interface
    // Emitter - object that can emitt signals from specified signal table
    // Transmitter - object that can be attach to emitter and can catch it's signals
    // Receiver - Implementation of reaction to the signals catched by transmitter
    //
    // General usecase of the library is to:
    // - Declare signal table as virtual interface with empty void functions
    // - Inherit YourClass from emitter<signal_table>
    // - Send signals like send(&signal_table::signal_name, args...);
    // - Inherit YourListenerClasses from receiver<signal_table> and implement signal reactions as
    //   override functions
    // - Do YourClass.attach(YourListenerClasses)
    //
    // More complicatd case could use packed signals - callable object, that could be stored,
    // tranfered and processed in muliple ways without detailed signal specification. For example
    // receiver could store packed signals in the queue to process them in the other thread.
    //
    // You cna attach multiple receivers to one emitter or one receiver to multiple emitters.
    // Multiple inheritance could be used to receive signals from emitters of different types.
    // Method send could return false in case if emitter instance is destroyed while sending.

    template<typename signal_table>
    class emitter;

    template<typename signal_table>
    using packed_signal = std::function<void(signal_table&)>;

    template<typename signal_table>
    class transmitter
        : private enable_spying
    {
        signal_table* _receiver = nullptr;
        friend class spy_pointer<transmitter<signal_table>>; // Provide access to parent

    public:
        transmitter(signal_table* receiver = nullptr) : _receiver(receiver) {}

        // Returns receiver's interface
        // If no receiver specified then packed signal will be transmitted
        signal_table* get() { return _receiver; }

        // Detach from all emitters
        void reset() { enable_spying::clear(); }
        // Check if it is attached to any emitter
        bool empty() { return enable_spying::empty(); }

        // Unpack packed signal and transmit it to the receiver
        // This method could be overrided to specify transmission
        virtual void transmit_signal(const packed_signal<signal_table>& call)
        {
            if (!_receiver)
                return;

            call(*_receiver);
        }
    };

    template<typename signal_table>
    class emitter
        : private enable_spying
    {
        using handle_type = spy_pointer<emitter<signal_table>>;
        using transmitter_type = transmitter<signal_table>;

        friend handle_type;

        std::vector<spy_pointer<transmitter_type>> _transmitters;

        void on_spying_state_changed()
        {
            // Do nothing in case if new broadcasting loop begins.
            if (sending())
                return;

            // Cleanup detached receivers int the receivers list when last recursive operation
            // finishes.
            _transmitters.erase(std::remove(_transmitters.begin(), _transmitters.end(), nullptr),
                             _transmitters.end());
        }

    public:
        void attach(transmitter_type& transmitter)
        {
            _transmitters.push_back(&transmitter);
        }

        void detach(transmitter_type& transmitter)
        {
            auto it = std::find(_transmitters.rbegin(), _transmitters.rend(), &transmitter);
            if (it == _transmitters.rend())
                return;

            // Hahdle spy pointer to self instance to cleanup transmitters list at the end of all
            // recursive broadcastin and detaching processes.
            handle_type self(this);
            it->reset();
        }

        bool empty()
        {
            for (auto& transmitter : _transmitters)
                if (transmitter)
                    return false;
            return true;
        }

        void reset()
        {
            // Hahdle spy pointer to self instance to cleanup transmitters list at the end of all
            // recursive broadcastin and detaching processes.
            handle_type self(this);

            for (auto& transmitter : _transmitters)
                transmitter.reset();
        }

        bool sending() { return !enable_spying::empty(); }

        template<typename FnType, typename... Args>
        bool send(FnType signal_name, Args&&... args)
        {
            // Hahdle spy pointer to self instance to catch self desctruction inside the
            // broadcasting loop.
            handle_type self(this);
            packed_signal<signal_table> call;

            // Do not use iterators here because _transmitters could grow.
            for (size_t i = 0; i < _transmitters.size(); ++i) {
                auto& transmitter = _transmitters[i];
                if (!transmitter)
                    continue;

                // Send signal directly if transmitter provedes receiver
                if (transmitter->get()) {
                    (transmitter->get()->*signal_name)(std::forward<Args>(args)...);
                }
                // Send packed signal in other case.
                else {
                    if (!call)
                        call = std::bind(signal_name, std::placeholders::_1, std::forward<Args>(args)...);
                    transmitter->transmit_signal(call);
                }

                // Break broadcasting loop in case if self destruction.
                if (!self)
                    return false;
            }
            return true;
        }

        bool send(const packed_signal<signal_table>& call)
        {
            if (!call)
                return true;
            
            // Hahdle spy pointer to self instance to catch self desctruction inside the
            // broadcasting loop.
            handle_type self(this);

            // Do not use iterators here because _transmitters could grow.
            for (size_t i = 0; i < _transmitters.size(); ++i) {
                auto& transmitter = _transmitters[i];
                if (!transmitter)
                    continue;

                // Send packed signal.
                transmitter->transmit_signal(call);

                // Break broadcasting loop in case of self destruction.
                if (!self)
                    return false;
            }
            return true;
        }
    };
}
