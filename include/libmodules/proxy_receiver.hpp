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

#include "emitter.hpp"
#include "lockable.hpp"
#include "spy_pointer.hpp"

#include <queue>
#include <mutex>

namespace mtl
{
    // This file is to implement signal transmission feature. Please see signal.hpp for details.
    // This file contains Proxy Receiver templates implementation that transfer signal received
    // from transmitter to another emitter and resend it.

    // Simple receiver implementation. It just implement signal table and perform signal
    // immediately.
    template<typename signal_table>
    class proxy_receiver
        : public transmitter<signal_table>
        , public emitter<signal_table>
    {
    public:
        virtual void transmit_signal(const packed_signal<signal_table>& call) { this->send(call); }
    };

    // Filter receiver is able to filter or change signal while transferring. It inherit
    // signal_table and performs signals before the transmission. That signal processing can
    // perform some computation with signal parameters, cancel transmission by filter_signal()
    // method and send signal with changed parameters.

    template<typename signal_table>
    class filter_proxy_receiver
        : public signal_table
        , public proxy_receiver<signal_table>
        , private enable_spying<filter_proxy_receiver<signal_table>>
    {
        using handle_type = spy_pointer<filter_proxy_receiver<signal_table>>;
        friend handle_type;

        bool _is_signal_filtered = false;

    protected:
        void filter_signal() { _is_signal_filtered = true; }

    public:
        virtual void transmit_signal(const packed_signal<signal_table>& call)
        {
            _is_signal_filtered = false;

            handle_type handle(this);
            call(*this);
            if (!handle) // Check if callback performed delete this
                return;

            if (!_is_signal_filtered)
                proxy_receiver<signal_table>::transmit_signal(call);
        }
    };

    // Queue receiver is able to store signals in a queue and transmit them later in the same
    // order. It could be used with lock system to separate emitting and receiving threads.
    // Warning: pointers and references stored in signal arguments could be invalidated before
    // the signal is executed. Your could send data by value or use shared pointer to handle
    // such situations.

    template<typename signal_table, typename locker_type = no_lockable>
    class queue_proxy_receiver
        : public transmitter<signal_table>
        , public emitter<signal_table>
    {
        std::queue<packed_signal<signal_table>> _signal_queue;
        locker_type _locker;

    public:
        queue_proxy_receiver(locker_type locker = locker_type()) : _locker(locker) {}
        queue_proxy_receiver(const queue_proxy_receiver& other) = default;
        queue_proxy_receiver(queue_proxy_receiver&& other) = default;


        virtual void transmit_signal(const packed_signal<signal_table>& call)
        {
            std::lock_guard<locker_type> lock(_locker);
            _signal_queue.push(call);
        }

        bool pop_signal()
        {
            packed_signal<signal_table> call;
            {
                std::lock_guard<locker_type> lock(_locker);
                if (_signal_queue.empty())
                    return false;

                call.swap(_signal_queue.front());
                _signal_queue.pop();
            } // ~lock_guard to release queue while call is transmitted
            this->send(call);
            return true;
        }
    };

} // namespace mtl
