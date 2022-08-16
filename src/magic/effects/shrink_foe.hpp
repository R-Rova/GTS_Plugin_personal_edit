#pragma once
#include "magic/magic.hpp"
// Module that handles footsteps


using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {
	class ShrinkFoe : public Magic {
		public:
			virtual void OnUpdate() override;

			static bool StartEffect(EffectSetting* effect);

			ShrinkFoe(ActiveEffect* effect);
		private:
			float _power = 0.0;
			float _efficiency = 0.0;
	};
}
