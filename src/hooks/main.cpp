#include "hooks/main.hpp"
#include "events.hpp"
#include "data/time.hpp"
#include "data/plugin.hpp"
#include "timer.hpp"
#include "Config.hpp"

using namespace RE;
using namespace SKSE;
using namespace Gts;

namespace Hooks
{

	void Hook_MainUpdate::Hook(Trampoline& trampoline)
	{
		REL::Relocation<uintptr_t> hook{REL::RelocationID(35551, 36544)};
		logger::info("Gts applying Main Update Hook at {:X}", hook.address());
		_Update = trampoline.write_call<5>(hook.address() + RELOCATION_OFFSET(0x11F, 0x160), Update);

		/*if (REL::Module::IsSE()) {
		        // auto offsetHelper = REL::IDDatabase::Offset2ID();
		        // log::info("Dumping OFFSETS");
		        // for (auto& offsetData: offsetHelper) {
		        // 	log::info("{}:{:X}:{}", offsetData.id, offsetData.offset, offsetData.offset);
		        // }

		        REL::Relocation<uintptr_t*> unknown_hook(REL::ID(38871), REL::Offset(0x3d4));
		        // REL::Relocation<uintptr_t> unknown_hook(REL::Offset(0x14067e824));
		        logger::info("Applying experimental hook: {:X}:{:X}", unknown_hook.address(), *unknown_hook.get());
		        _UnknownMaybeScale = trampoline.write_call<5>(unknown_hook.address(), UnknownMaybeScale);
		        logger::info("  - Applied experimental hook");
		   }*/
	}

	void Hook_MainUpdate::Update(RE::Main* a_this, float a2)
	{
		_Update(a_this, a2);
		Time::GetSingleton().Update();

		static std::atomic_bool started = std::atomic_bool(false);
		Plugin::SetOnMainThread(true);
		if (Plugin::Live()) {
			// We are not loading or in the mainmenu
			// Player loaded and not paused
			if (started.exchange(true)) {
				// Not first updated
				EventDispatcher::DoUpdate();
			} else {
				// First update this load
				EventDispatcher::DoStart();
			}
		} else if (!Plugin::InGame()) {
			// Loading or in main menu
			started.store(false);
		}
		Plugin::SetOnMainThread(false);

		if (Config::GetSingleton().GetDebug().ShouldProfile()) {
			static Timer timer = Timer(5.0);
			if (timer.ShouldRun()) {
				EventDispatcher::ReportProfilers();
			}
		}
	}

	void Hook_MainUpdate::UnknownMaybeScale(UnknownMaybeScaleObject* unknown_a, float a1) {
		// Prints nothing at all.
		log::info("UnknownMaybeScale");
		if (unknown_a) {
			log::info("unknown_a: {}", GetRawName(unknown_a));
		} else {
			log::info("unknown_a: Nullprt");
		}
		log::info("unknown_b: {}", a1);

		// _UnknownMaybeScale(unknown_a, a1);
	}
	void Hook_MainUpdate::UnknownMaybeScale2(Actor* actor, float scale) {
		log::info("UnknownMaybeScale2"); //<------ Prints it about 5 times, then crashes on save load
		//// ^ isn't printing anything below that. Tried both UnknownMaybeScaleObject* unknown_a and Actor* actor.
		log::info("Actor: {}", actor->GetDisplayFullName());
		log::info("Scale: {}", scale);
		//log::info("scale: {}", a2);

		// _UnknownMaybeScale(unknown_a, a1, a2);
	}
}
