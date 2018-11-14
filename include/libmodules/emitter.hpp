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

#include "signal.hpp"

#include <vector>
#include <algorithm>

namespace mtl
{
    // This file is to implement signal transmission feature. Please see signal.hpp for details.
    // This file contains Emitter template implementation that emit signals to attached
    // transmitters.

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
            size_t count = _transmitters.size();
            for (size_t i = 0; i < count; ++i) {
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
} // namespace mtl
