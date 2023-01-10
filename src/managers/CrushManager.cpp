#include "managers/CrushManager.hpp"
#include "magic/effects/common.hpp"
#include "data/runtime.hpp"
#include "data/time.hpp"
#include "scale/scale.hpp"
#include "actorUtils.hpp"
#include "papyrusUtils.hpp"
#include "util.hpp"

#include <random>

using namespace SKSE;
using namespace RE;
using namespace REL;
using namespace Gts;

namespace {
	void ScareChance(Actor* actor) {
		int voreFearRoll = rand() % 5;
		if (Runtime::HasMagicEffect(actor, "SmallMassiveThreat")) {
			voreFearRoll = rand() % 2;
			shake_camera(actor, 0.4, 0.25);
		}

		if (voreFearRoll <= 0) {
			Runtime::CastSpell(actor, actor, "GtsVoreFearSpell");
			KnockAreaEffect(actor, 6, 60 * get_target_scale(actor));
		}
	}

	void FearChance(Actor* giant)  {
		float size = get_visual_scale(giant);
		int MaxValue = (20 - (1.6 * size));

		// TODO: Ask about the max value thing here
		// If you have small massive threat then the max value is ALWAYS 4

		// S.Answer: It's supposed to proc more often with SMT active, so having it always 4 is fine ^
		if (MaxValue <= 4 || Runtime::HasMagicEffect(giant, "SmallMassiveThreat")) {
			MaxValue = 4;
		}
		int FearChance = rand() % MaxValue;
		if (FearChance <= 0 ) {
			//auto event = RegistrationSet("CastFear");
			//event.SendEvent();
			Runtime::CastSpell(giant, giant, "GtsVoreFearSpell");
			// Should cast fear
		}
	}

	void PleasureText(Actor* actor) {
		int Pleasure = rand() % 5;
		if (Pleasure <= 0) {
			if (actor->formID == 0x14) {
				Notify("Crushing your foes feels good and makes you bigger");
			} else {
				Notify("Your companion grows bigger by crushing your foes");
			}
		}
	}

	void GrowAfterTheKill(Actor* caster, Actor* target) {
		if (!Runtime::GetBool("GtsDecideGrowth") || Runtime::HasMagicEffect(caster, "SmallMassiveThreat")) {
			return;
		} else if (Runtime::HasPerk(PlayerCharacter::GetSingleton(), "GrowthPerk") && Runtime::GetInt("GtsDecideGrowth") >= 1) {
			float Rate = (0.00020 * get_target_scale(target)) * 120;
			if (Runtime::HasPerk(PlayerCharacter::GetSingleton(), "AdditionalAbsorption")) {
				Rate *= 2.0;
			}
			CrushGrow(caster, 0, Rate);
			log::info("Caster: {}, Growth Value: {}", caster->GetDisplayFullName(), Rate);
		}
		PleasureText(caster);
	}

	void RandomMoan(Actor* caster, Actor* target) {
		auto randomInt = rand() % 10;
		if (randomInt < 1 ) {
			Runtime::PlaySound("MoanSound", caster, 1.0, 1.0);
			GrowAfterTheKill(caster, target);
		}
	}

	void AdjustGiantessSkill(Actor* Caster, Actor* Target) {
		if (Caster->formID !=0x14) {
			return; //Bye
		}
		auto GtsSkillLevel = Runtime::GetGlobal("GtsSkillLevel");
		auto GtsSkillRatio = Runtime::GetGlobal("GtsSkillRatio");
		auto GtsSkillProgress = Runtime::GetGlobal("GtsSkillProgress");

		int random = (100 + (rand()% 50 + 1)) / 100;

    	if (GtsSkillLevel->value >= 100;) {
       	 	GtsSkillLevel->value = 100.0;
        	GtsSkillRatio->value = 0.0;
        	return;
    	}
    
    	float ValueEffectiveness = clamp(0.10, 1.0, 1.0 - GtsSkillLevel->value);

    	float absorbedSize = (get_target_scale(Target)) + (Target->GetLevel() * 4.0);
    	float Total = (((0.14 * random) + absorbedSize/50) * ValueEffectiveness);
   		GtsSkillRatio->value += Total; 
		int TotalLevel = GtsSkillLevel->value;
    
    	if (GtsSkillRatio->value >= 1.0) {
        	GtsSkillLevel->value += 1.0;
        	GtsSkillProgress->value = TotalLevel;
        	GtsSkillRatio->value = 0.0;
		}
	}
}

namespace Gts {
	CrushManager& CrushManager::GetSingleton() noexcept {
		static CrushManager instance;
		return instance;
	}

	std::string CrushManager::DebugName() {
		return "CrushManager";
	}

	void CrushManager::Update() {
		for (auto &[tiny, data]: this->data) {
			auto giant = data.giant;
			if (!tiny) {
				continue;
			}
			if (!giant) {
				continue;
			}
			auto progressionQuest = Runtime::GetQuest("MainQuest");

			if (data.state == CrushState::Healthy) {
				RandomMoan(data.giant, tiny);
				data.state = CrushState::Crushing;
			} else if (data.state == CrushState::Crushing) {
				if (data.delay.ShouldRun()) {
					data.state = CrushState::Crushed;

					// Do crush
					Runtime::PlaySound("GtsCrushSound", giant, 1.0, 1.0);
					Runtime::PlaySound("GtsFallSound", giant, 1.0, 1.0);
					Runtime::CastSpell(tiny, tiny, "GtsBleedSpell");
					AdjustGiantessSkill(giant, tiny);
					GrowAfterTheKill(giant, tiny);
					if (giant->formID == 0x14 && IsDragon(tiny)) {
						if (progressionQuest) {
							CallFunctionOn(progressionQuest, "Quest", "DevourDragon");
						}
					}
					shake_camera(giant, 1.8, 1);
					if (giant->formID == 0x14) {   
						TriggerScreenBlood(1);
					}
					std::random_device rd;
					std::mt19937 gen(rd());
					std::uniform_real_distribution<float> dis(-0.2, 0.2);

					Runtime::PlayImpactEffect(tiny, "GtsBloodSprayImpactSet", "NPC Head", NiPoint3{dis(gen), 0, -1}, 512, true, true);
					Runtime::PlayImpactEffect(tiny, "GtsBloodSprayImpactSet", "NPC L Foot [Lft]", NiPoint3{dis(gen), 0, -1}, 512, true, false);
					Runtime::PlayImpactEffect(tiny, "GtsBloodSprayImpactSet", "NPC R Foot [Rft]", NiPoint3{dis(gen), 0, -1}, 512, true, false);
					if (giant->formID == 0x14 && Runtime::GetBool("GtsEnableLooting")) {
						Actor* into = giant;
						TransferInventory(tiny, into, false, true);
					} else if (giant->formID != 0x14 && Runtime::GetBool("GtsNPCEnableLooting")) {
						Actor* into = giant;
						TransferInventory(tiny, into, false, true);
					}
					ScareChance(giant);

					if (tiny->formID != 0x14) {
						Disintegrate(tiny); // CTD if we Disintegrate the player	
					}

					FearChance(giant);
				}
			}
		}
	}

	void CrushManager::Reset() {
		this->data.clear();
	}

	void CrushManager::ResetActor(Actor* actor) {
		if (actor) {
			UnDisintegrate(actor);
			SetUnRestrained(actor);
			SetMove(actor);
		}
		this->data.erase(actor);
	}

	void CrushManager::Crush(Actor* giant, Actor* tiny) {
		if (CrushManager::CanCrush(giant, tiny)) {
			CrushManager::GetSingleton().data.try_emplace(tiny, giant, tiny);
		}
	}

	bool CrushManager::AlreadyCrushed(Actor* actor) {
		auto& m = CrushManager::GetSingleton().data;
		return (m.find(actor) != m.end());
	}

	bool CrushManager::CanCrush(Actor* giant, Actor* tiny) {
		if (CrushManager::AlreadyCrushed(tiny)) {
			return false;
		}

		// Check if they are immune
		const std::string_view CANT_CRUSH_EDITORID = "GtsCantStomp";
		if (tiny->HasKeywordString(CANT_CRUSH_EDITORID)) {
			// TODO: Check GtsCantStomp is a valid keyword
			return false;
		}

		// Check skin
		auto skin = tiny->GetSkin();
		if (skin) {
			if (skin->HasKeywordString(CANT_CRUSH_EDITORID)) {
				return false;
			}
		}
		const auto inv = tiny->GetInventory([](TESBoundObject& a_object) {
			return a_object.IsArmor();
		});

		// Check worn armor
		for (const auto& [item, invData] : inv) {
			const auto& [count, entry] = invData;
			if (count > 0 && entry->IsWorn()) {
				const auto armor = item->As<TESObjectARMO>();
				if (armor && armor->HasKeywordString(CANT_CRUSH_EDITORID)) {
					return false;
				}
			}
		}

		return true;
	}

	CrushData::CrushData(Actor* giant, Actor* tiny) :
		delay(Timer(0.01)),
		state(CrushState::Healthy),
		giant(giant) {
	}
}
