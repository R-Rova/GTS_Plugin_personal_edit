#pragma once
// Module that holds data that is persistent across saves
#include <SKSE/SKSE.h>
#include "scale/modscale.h"
#include "util.h"

using namespace std;
using namespace SKSE;
using namespace RE;
using namespace Gts;

namespace Gts {
	struct ActorData {
		float native_scale;
		float visual_scale;
		float visual_scale_v;
		float target_scale;
		float max_scale;
		float half_life;
		float anim_speed;
		float effective_multi;
	};

	class Persistent {
		public:
			[[nodiscard]] static Persistent& GetSingleton() noexcept;
			static void OnRevert(SKSE::SerializationInterface*);
			static void OnGameSaved(SKSE::SerializationInterface* serde);
			static void OnGameLoaded(SKSE::SerializationInterface* serde);

			ActorData* GetActorData(Actor* actor);
			ActorData* GetData(TESObjectREFR* refr);

			bool highheel_correction = true;
			bool is_speed_adjusted = true;
			SoftPotential speed_adjustment {
				.k = 0.125,
				.n = 0.85,
				.s = 1.12,
				.o = 1.0,
			};
			SizeMethod size_method = SizeMethod::ModelScale;
		private:
			Persistent() = default;

			mutable std::mutex _lock;
			std::unordered_map<FormID, ActorData> _actor_data;
	};
}
