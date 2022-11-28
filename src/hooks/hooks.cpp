#include "hooks/hooks.hpp"
#include "hooks/main.hpp"
#include "hooks/headTracking.hpp"
#include "hooks/impact.hpp"
#include "hooks/vm.hpp"
#include "hooks/havok.hpp"
#include "hooks/magicTarget.hpp"
#include "hooks/hkbBehaviorGraph.hpp"
#include "hooks/playerCamera.hpp"
#include "hooks/cameraState.hpp"

using namespace RE;

namespace Hooks
{
	void Install()
	{
		logger::info("Gts applying hooks...");

		auto& trampoline = SKSE::GetTrampoline();
		trampoline.create(256);

		Hook_MainUpdate::Hook(trampoline);
		Hook_BGSImpactManager::Hook();
		Hook_VM::Hook();
		Hook_Havok::Hook(trampoline);
		Hook_HeadTracking::Hook(trampoline);
		//Hook_MagicTarget::Hook();
		Hook_hkbBehaviorGraph::Hook();
		Hook_PlayerCamera::Hook();
		Hook_CameraState::Hook();

		logger::info("Gts finished applying hooks...");
	}
}
