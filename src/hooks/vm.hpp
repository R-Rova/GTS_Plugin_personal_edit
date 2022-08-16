#pragma once
#include "hooks/hooks.hpp"

using namespace RE;
using namespace SKSE;

using namespace RE;
using namespace RE::BSScript;
using namespace RE::BSScript::Internal;

namespace Hooks
{
	class HookVm
	{
		public:
			static void Hook();
		private:
			static void SendEvent(VirtualMachine* a_this, VMHandle a_handle, const BSFixedString& a_event_name, IFunctionArguments* a_args);
			static inline REL::Relocation<decltype(SendEvent)> send_event;
	};
}
