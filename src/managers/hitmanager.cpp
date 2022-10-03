#include "managers/hitmanager.hpp"
#include "managers/GtsSizeManager.hpp"
#include "managers/GrowthTremorManager.hpp"
#include "magic/effects/common.hpp"
#include "managers/GtsManager.hpp"
#include "data/persistent.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"
#include "data/time.hpp"
#include "timer.hpp"
#include "util.hpp"
#include "node.hpp"

using namespace RE;
using namespace Gts;


namespace Gts {
	HitManager& HitManager::GetSingleton() noexcept {
		static HitManager instance;
		return instance;
	}

	void HitManager::HitEvent(const TESHitEvent* a_event) {
		if (!a_event) {
			return;
		}
		auto attacker_PTR = a_event->cause;
		auto atacker_ref = attacker_PTR.get();
		auto attacker = skyrim_cast<Actor*>(atacker_ref);
		if (!attacker) {
			return;
		}
		auto receiver_PTR = a_event->target;
		auto receiver_ref = receiver_PTR.get();
		auto receiver = skyrim_cast<Actor*>(receiver_ref);
		if (!receiver) {
			return;
		}
		auto runtime = Runtime::GetSingleton();
		auto sizemanager = SizeManager::GetSingleton();
		auto Persist = Persistent::GetSingleton();
		auto HitIdForm = a_event->source;
		auto HitId = TESForm::LookupByID(HitIdForm);

		auto ProjectileIDForm = a_event->projectile;
		auto ProjectileID = TESForm::LookupByID(ProjectileIDForm);
		auto player = PlayerCharacter::GetSingleton();

		bool wasPowerAttack = a_event->flags.all(TESHitEvent::Flag::kPowerAttack);
		bool wasSneakAttack = a_event->flags.all(TESHitEvent::Flag::kSneakAttack);
		bool wasBashAttack = a_event->flags.all(TESHitEvent::Flag::kBashAttack);
		bool wasHitBlocked = a_event->flags.all(TESHitEvent::Flag::kHitBlocked);

		// Do something
		
		if (receiver == player && receiver->HasPerk(runtime.GrowthOnHitPerk) && HitId->GetName() != "Stagger" && sizemanager.GetHitGrowth(receiver) < 0.01) {
			ConsoleLog::GetSingleton()->Print("First condition passed");
			if(wasHitBlocked == false && attacker->IsPlayerTeammate() == false && attacker != player) {
				if (wasPowerAttack) {
					this->BonusPower = 3.0;
				}
				else if (!wasPowerAttack) {
					this->BonusPower = 1.0;
				}
				ConsoleLog::GetSingleton()->Print("Hit Initialized.");
				float ReceiverScale = get_visual_scale(receiver);
				float DealerScale = get_visual_scale(attacker);
				float HealthMult = GetMaxAV(receiver, ActorValue::kHealth) / receiver->GetActorValue(ActorValue::kHealth);
				float HealthPercentage = GetHealthPercentage(receiver);
				float SizeDifference = ReceiverScale/DealerScale;
				float LaughChance = rand() % 12;
				auto GrowthSound = runtime.growthSound;

				auto actor_data = Persist.GetData(receiver);
				actor_data->half_life = 1.0/HealthPercentage;

				PlaySound(GrowthSound, receiver, ReceiverScale/20, 0.0);

				this->CanGrow = true;
				this->GrowthTick +=GetHealthPercentage(receiver);

				log::info("GetGrowthTime of {} is {}, HP mult: {}", receiver->GetDisplayFullName(), sizemanager.GetGrowthTime(receiver), HealthMult);
				log::info("GetHitGrowth of {} is {}", receiver->GetDisplayFullName(), sizemanager.GetHitGrowth(receiver));
				if (SizeDifference >= 4.0 && LaughChance >= 12.0) {
					auto LaughSound = Runtime::GetSingleton().LaughSound;
					PlaySound(LaughSound, receiver, 1.0, 0.0); //FearCast()
				}
			}
		}
	}

	void HitManager::Update() {
			auto actor = PlayerCharacter::GetSingleton();
			auto Runtime = Runtime::GetSingleton();
			auto sizemanager = SizeManager::GetSingleton();
			auto Persist = Persistent::GetSingleton();
			if (this->CanGrow) {
				ConsoleLog::GetSingleton()->Print("GetHitGrowth > 0");
				float HealthMult = GetMaxAV(actor, ActorValue::kHealth) / actor->GetActorValue(ActorValue::kHealth);
				float GrowthValue = HealthMult/9700/this->BonusPower;
				
				if (actor->HasMagicEffect(Runtime.SmallMassiveThreat)) {
					GrowthValue *= 0.50;
				}
				if (this->GrowthTick > 0.01) {
					ConsoleLog::GetSingleton()->Print("Growth Started.");
					GrowthTremorManager::GetSingleton().CallRumble(actor, actor, actor_data->half_life * 2);
					mod_target_scale(actor, GrowthValue * (get_visual_scale(actor) * 0.25 + 0.75));
					this->GrowthTick -= 0.001 * TimeScale();
				} else if (this->GrowthTick < 0.01) {
					
					auto actor_data = Persist.GetData(receiver);
					actor_data->half_life = 1.0;
					this->CanGrow = false;
					this->GrowthTick = 0.0;
					ConsoleLog::GetSingleton()->Print("Growth Ended.");
			}
		}
	}
}

