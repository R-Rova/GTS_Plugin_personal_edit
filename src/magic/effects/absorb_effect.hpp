#pragma once
#include "magic/magic.hpp"
// Module that handles footsteps


using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {
	class Absorb : public Magic {
		public:
			Absorb(ActiveEffect* effect);

			virtual void OnUpdate() override;

			static bool StartEffect(EffectSetting* effect);
		private:
			bool _true_absorb = false;
	};
}
