/******************************************************************************

Project libmodule
Copyright 2018 Sergey Ushakov

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

#include "spy_pointer.hpp"

namespace modules
{
  template<typename SignalTable>
  class emitter;

  template<typename SignalTable>
  using signal = std::function<void(const SignalTable&)>;

  template<typename SignalTable>
  class receiver_base
    : private enable_spying
  {
  protected:
    receiver_base(SignalTable* executor = nullptr) : _p_executor(executor) {}
    virtual void process_signal(const signal<SignalTable>& call)
    {
      assert(_p_executor);
      call(*_p_executor);
    }

    friend class emitter<SignalTable>;
    SignalTable* _p_executor = nullptr;
  };

  template<typename SignalTable>
  class emitter
    : private enable_spying
  {
  public:
    void attach(receiver_base<SignalTable>& receiver)
    {
      _receivers.push_back(&receiver);
    }

    void detach(receiver_vase<SignalTable>& receiver)
    {
      auto it = std::find(_receivers.rbegin(), _receivers.rend(), *receiver);
      if (it == _receivers.rend())
	return;
      _receivers.erase(it.base()-1);
    }

    template<typename FnType, typename Args...>
    void Send(FnTyep signal_name, Args&&... args...)
    {
      spy_pointer<Type> self(this);
      SignalTable call;

      for (auto& p_receiver : _receivers)
      {
	if (!p_receiver)
	  continue;

	if (p_receiver->_p_executor)
	{
	  
    }

    void Send(const SignalType& call)
    {
      spy_pointer<Type> self(this);
      for (auto& p_receiver : _receivers)
      {
	if (!p_receiver)
	  continue;

	p_receiver->process_signal(call);
      }
    }

  private:
    using Type = emitter<SignalTable>;
    using SignalType = signal<SignalTable>;
    template<typename PointerType>
    friend class spy_pointer;

    void on_spying_state_changed()
    {
      if (is_spied())
	return;

      _receivers.erase(std::remove(_receivers.begin(), _receivers.end(), nullptr), _receivers.end());
    }
}
