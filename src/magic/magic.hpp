#pragma once
// Module that handles footsteps
#include "events.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {
	template <class T>
	T* find_form(std::string_view lookup_id) {
		// From https://github.com/Exit-9B/MCM-Helper/blob/a39b292909923a75dbe79dc02eeda161763b312e/src/FormUtil.cpp
		std::string lookup_id_str(lookup_id);
		std::istringstream ss{ lookup_id_str };
		std::string plugin, id;

		std::getline(ss, plugin, '|');
		std::getline(ss, id);
		RE::FormID relativeID;
		std::istringstream{ id } >> std::hex >> relativeID;
		const auto dataHandler = RE::TESDataHandler::GetSingleton();
		return dataHandler ? dataHandler->LookupForm<T>(relativeID, plugin) : nullptr;
	}

	class Magic {
		public:
			virtual void OnStart();
			virtual void OnUpdate();
			virtual void OnFinish();

			virtual std::string GetName();

			void poll();

			Actor* GetTarget();
			Actor* GetCaster();

			ActiveEffect* GetActiveEffect();
			EffectSetting* GetBaseEffect();

			void Dispel();
			bool IsDualCasting();
			inline bool DualCasted() {
				return this->dual_casted;
			}
			bool HasDuration();

			Magic(ActiveEffect* effect);

			inline bool IsFinished() {
				return this->state == State::CleanUp;
			}

		private:
			enum State {
				Init,
				Start,
				Update,
				Finish,
				CleanUp,
			};
			State state = State::Init;
			Actor* target = nullptr;
			Actor* caster = nullptr;
			ActiveEffect* activeEffect = nullptr;
			EffectSetting* effectSetting = nullptr;
			bool dual_casted = false;
			bool hasDuration = false;
	};

	template<class MagicCls>
	class MagicFactory<MagicCls> {
		MagicCls* MakeNew(EffectSetting* effect) {
			if (effect) {
				return new MagicCls(effect);
			} else {
				return nullptr;
			}
		}
	};

	class MagicManager : public EventListener {
		public:


			[[nodiscard]] static MagicManager& GetSingleton() noexcept;

			virtual std::string DebugName() override;
			virtual void Update() override;
			virtual void Update() override;
			virtual void Reset() override;
			virtual void DataReady() override;

			void ProcessActiveEffects(Actor* actor);

			void RegisterMagic<MagicCls>(std::string_view tag) {
				auto magic = Runtime::GetMagicEffect(tag);
				if (tag) {
					this->factories.try_emplace(magic,MagicFactory<MagicCls>());
				}
			}
		private:
			std::map<ActiveEffect*, std::unique_ptr<Magic> > active_effects;
			std::unordered_map<EffectSetting*, std::unique_ptr<MagicFactory> > factories;
	};
}
