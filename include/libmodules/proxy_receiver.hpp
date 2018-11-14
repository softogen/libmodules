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

    // Filter receiver is able to filter or change signal while transfering. It inherit
    // signal_table and perfos signals before the transmission. That signal processing can perform
    // some computation with signal parameters, cancel transmission by calncel() method and resend
    // signal with changed parameters.

} // namespace mtl
