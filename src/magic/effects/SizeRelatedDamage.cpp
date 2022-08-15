#include "magic/effects/shrink.hpp"
#include "magic/effects/common.hpp"
#include "magic/magic.hpp"
#include "scale/scale.hpp"
#include "data/runtime.hpp"

namespace Gts {
	bool SizeDamage::StartEffect(EffectSetting* effect) {
        auto& runtime = Runtime::GetSingleton();
		return (effect == runtime.SizeRelatedDamage0 || effect == runtime.SizeRelatedDamage1 || effect == runtime.SizeRelatedDamage2);
	}

	void SizeDamage::OnUpdate() {
		auto caster = GetCaster();
		if (!caster) {
			return;
		}
		auto target = GetTarget();
		if (!target) {
			return;
		}
        CallCrush = caster->GetMagicCaster(RE::MagicSystem::CastingSource::kInstant)->CastSpellImmediate(runtime.FakeCrushSpell, false, target, 1.00f, false, 0.0f, caster)
		auto& runtime = Runtime::GetSingleton();
		float size_limit = runtime.sizeLimit->value;
		float ProgressionMultiplier = runtime.ProgressionMultiplier->value;
		float casterScale = get_visual_scale(caster);
        float targetScale = get_visual_scale(target);
		float DualCast = 1.0;
        float SizeDifference = casterScale/targetScale;
        if (target->IsPlayerTeammate() == true && runtime.GtsNPCEffectImmunityToggle->value == 1.0)
        {return;} // Do not apply if those are true
        if (SizeDifference >= 4.0 && target->IsDead() && target->IsPlayerTeammate() == false) // We don't want to crush allies
        {CallCrush();} // < Crush only if size difference is > than 4.0
        else if (SizeDifference >= 24.0)
        {CallCrush();} // < Crush anyway, no conditions needed since size difference is too massive
	}
}