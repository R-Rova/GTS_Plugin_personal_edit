#include "managers/GrowthTremorManager.hpp"
#include "managers/GtsManager.hpp"
#include "magic/effects/GrowthPotion.hpp"
#include "magic/effects/common.hpp"
#include "magic/magic.hpp"
#include "scale/scale.hpp"
#include "data/runtime.hpp"
#include "util.hpp"
#include "timer.hpp"

namespace Gts {
	std::string GrowthPotion::GetName() {
		return "GrowthPotion";
	}

	bool GrowthPotion::StartEffect(EffectSetting* effect) { // NOLINT
		auto& runtime = Runtime::GetSingleton();
		return effect == runtime.EffectGrowthPotion;
	}

	void GrowthPotion::OnStart() {
		auto caster = GetCaster();
		auto player = PlayerCharacter::GetSingleton();
		auto& runtime = Runtime::GetSingleton();
		if (!caster || !player) {
			return;
		}
		GrowthTremorManager::GetSingleton().CallRumble(caster, player, 2.0);

		auto GrowthSound = runtime.growthSound;
		float Volume = clamp(0.15, 2.0, get_visual_scale(caster)/8);
		PlaySound_Frequency(GrowthSound, caster, Volume, 1.0);
		log::info("Growth Potion start actor: {}", caster->GetDisplayFullName());
	}

	void GrowthPotion::OnUpdate() {
		const float BASE_POWER = 0.000180;
		auto& runtime = Runtime::GetSingleton();


		auto caster = GetCaster();
		if (!caster) {
			return;
		}

		float AlchemyLevel = clamp(1.0, 2.0, caster->GetActorValue(ActorValue::kAlchemy)/100 + 1.0);
		if (caster == PlayerCharacter::GetSingleton()) {
			GrowthTremorManager::GetSingleton().CallRumble(caster, caster, 0.4);
		}

		if (this->timer.ShouldRun()) {
			auto GrowthSound = runtime.growthSound;
			float Volume = clamp(0.15, 1.0, get_visual_scale(caster)/4);
			PlaySound_Frequency(GrowthSound, caster, Volume, 1.0);
			log::info("Growth Potion Loop caster: {}", caster->GetDisplayFullName());
		}
		float HP = caster->GetPermanentActorValue(ActorValue::kHealth) * 0.00085;
		caster->RestoreActorValue(ACTOR_VALUE_MODIFIER::kDamage, ActorValue::kHealth, HP * TimeScale());

		float Power = BASE_POWER * get_visual_scale(caster) * AlchemyLevel;

		Grow(caster, 0.0, Power);
	}

	void GrowthPotion::OnFinish() {
	}
}
