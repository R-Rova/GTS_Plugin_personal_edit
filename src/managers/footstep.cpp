#include "managers/footstep.h"
#include "managers/impact.h"

#include "scale/scale.h"
#include "managers/modevent.h"
#include "util.h"
#include "node.h"
#include "data/runtime.h"

using namespace SKSE;
using namespace RE;
using namespace Gts;

namespace {
	struct VolumeParams {
		float a;
		float k;
		float n;
		float s;
	};

	float volume_function(float scale, const VolumeParams& params) {
		float k = params.k;
		float a = params.a;
		float n = params.n;
		float s = params.s;
		// https://www.desmos.com/calculator/ygoxbe7hjg
		return k*pow(s*(scale-a), n);
	}

	float frequency_function(float scale, const VolumeParams& params) {
		float a = params.a;
		return soft_core(scale, 0.01, 1.0, 1.0, a)*0.5+0.5;
	}

	float falloff_function(NiAVObject* source) {
		if (source) {
			float distance_to_camera = unit_to_meter(get_distance_to_camera(source));
			// Camera distance based volume falloff
			return soft_core(distance_to_camera, 0.024, 2.0, 0.8, 0.0);
		}
		return 1.0;
	}

	BSSoundHandle get_sound(NiAVObject* foot, const float& scale, BSISoundDescriptor* sound_descriptor, const VolumeParams& params, std::string_view tag) {
		BSSoundHandle result = BSSoundHandle::BSSoundHandle();
		auto audio_manager = BSAudioManager::GetSingleton();
		if (sound_descriptor && foot && audio_manager) {

			float volume = volume_function(scale, params);
			float frequency = frequency_function(scale, params);
			float falloff = falloff_function(foot);
			float intensity = volume * falloff;

			if (intensity > 1e-5) {
				// log::trace("  - Playing {} with volume: {}, falloff: {}, intensity: {}", tag, volume, falloff, intensity);
				audio_manager->BuildSoundDataFromDescriptor(result, sound_descriptor);
				result.SetVolume(intensity);
				result.SetFrequency(frequency);
				NiPoint3 pos;
				pos.x = 0;
				pos.y = 0;
				pos.z = 0;
				result.SetPosition(pos);
				result.SetObjectToFollow(foot);
			}
		}
		return result;
	}

	BSISoundDescriptor* get_lFootstep_sounddesc(const Foot& foot_kind) {
		switch (foot_kind) {
			case Foot::Left:
			case Foot::Front:
				return Runtime::GetSingleton().lFootstepL;
				break;
			case Foot::Right:
			case Foot::Back:
				return Runtime::GetSingleton().lFootstepR;
				break;
		}
		return nullptr;
	}

	BSISoundDescriptor* get_lJumpLand_sounddesc(const Foot& foot_kind) {
		switch (foot_kind) {
			case Foot::JumpLand:
				return Runtime::GetSingleton().lJumpLand;
				break;
		}
		return nullptr;
	}

	BSISoundDescriptor* get_xlFootstep_sounddesc(const Foot& foot_kind) {
		switch (foot_kind) {
			case Foot::Left:
			case Foot::Front:
				return Runtime::GetSingleton().xlFootstepL;
				break;
			case Foot::Right:
			case Foot::Back:
				return Runtime::GetSingleton().xlFootstepR;
				break;
		}
		return nullptr;
	}

	BSISoundDescriptor* get_xlRumble_sounddesc(const Foot& foot_kind) {
		switch (foot_kind) {
			case Foot::Left:
			case Foot::Front:
				return Runtime::GetSingleton().xlRumbleL;
				break;
			case Foot::Right:
			case Foot::Back:
				return Runtime::GetSingleton().xlRumbleR;
				break;
			case Foot::JumpLand:
				return Runtime::GetSingleton().xlRumbleR;
				break;
		}
		return nullptr;
	}

	BSISoundDescriptor* get_xlSprint_sounddesc(const Foot& foot_kind) {
		switch (foot_kind) {
			case Foot::Left:
			case Foot::Front:
				return Runtime::GetSingleton().xlSprintL;
				break;
			case Foot::Right:
			case Foot::Back:
				return Runtime::GetSingleton().xlSprintR;
				break;
			case Foot::JumpLand:
				return Runtime::GetSingleton().xlSprintR;
				break;
		}
		return nullptr;
	}

	BSISoundDescriptor* get_xxlFootstep_sounddesc(const Foot& foot_kind) {
		switch (foot_kind) {
			case Foot::Left:
			case Foot::Front:
				return Runtime::GetSingleton().xxlFootstepL;
				break;
			case Foot::Right:
			case Foot::Back:
				return Runtime::GetSingleton().xxlFootstepR;
				break;
			case Foot::JumpLand:
				return Runtime::GetSingleton().xxlFootstepR;
				break;
		}
		return nullptr;
	}
}
namespace Gts {
	FootStepManager& FootStepManager::GetSingleton() noexcept {
		static FootStepManager instance;
		return instance;
	}

	void FootStepManager::OnImpact(const Impact& impact) {
		if (impact.actor) {
			auto actor = impact.actor;
			float scale = impact.effective_scale;

			float minimal_size = 1.2;
			if (scale > minimal_size && !actor->IsSwimming()) {
				float start_l = 1.2;
				float start_xl = 11.99;
				float start_xlJumpLand= 1.99;
				float start_xxl = 20.0;
				if (actor->IsSprinting()) {
					float sprint_factor = 1.2;
					scale *= sprint_factor; // Sprinting makes you sound bigger
					start_xl = 7.99 * sprint_factor;
					start_xxl = 15.99 * sprint_factor;
				} else if (actor->IsSneaking()) {
					scale *= 0.55; // Sneaking makes you sound quieter
				} else if (actor->IsWalking()) {
					scale *= 0.85; // Walking makes you sound quieter
				}
				Foot foot_kind = impact.kind;
				if (foot_kind == Foot::JumpLand) {
					float jump_factor = 1.2;
					scale *= jump_factor; // Jumping makes you sound bigger
					start_xl = 6.99 * jump_factor;
					start_xxl = 14.99 * jump_factor;
				}
				for (NiAVObject* foot: impact.nodes) {
					BSSoundHandle lFootstep    = get_sound(foot, scale, get_lFootstep_sounddesc(foot_kind),   VolumeParams { .a = start_l,             .k = 0.6,  .n = 0.7, .s = 1.0}, "L Footstep");
					BSSoundHandle lJumpLand    = get_sound(foot, scale, get_lJumpLand_sounddesc(foot_kind),   VolumeParams { .a = start_l,             .k = 0.6,  .n = 0.7, .s = 1.0}, "L Jump");

					BSSoundHandle xlFootstep   = get_sound(foot, scale, get_xlFootstep_sounddesc(foot_kind),  VolumeParams { .a = start_xl,            .k = 0.65, .n = 0.5, .s = 1.0}, "XL: Footstep");
					BSSoundHandle xlRumble     = get_sound(foot, scale, get_xlRumble_sounddesc(foot_kind),    VolumeParams { .a = start_xl,            .k = 0.65, .n = 0.5, .s = 1.0}, "XL Rumble");
					BSSoundHandle xlSprint     = get_sound(foot, scale, get_xlSprint_sounddesc(foot_kind),    VolumeParams { .a = start_xl,            .k = 0.65, .n = 0.5, .s = 1.0}, "XL Sprint");

					BSSoundHandle xxlFootstepL = get_sound(foot, scale, get_xxlFootstep_sounddesc(foot_kind), VolumeParams { .a = start_xxl,           .k = 0.6,  .n = 0.5, .s = 1.0}, "XXL Footstep");


					if (lFootstep.soundID != BSSoundHandle::kInvalidID) {
						lFootstep.Play();
					}
					if (lJumpLand.soundID != BSSoundHandle::kInvalidID) {
						lJumpLand.Play();
					}
					if (xlFootstep.soundID != BSSoundHandle::kInvalidID) {
						xlFootstep.Play();
					}
					if (xlRumble.soundID != BSSoundHandle::kInvalidID) {
						xlRumble.Play();
					}
					if (xlSprint.soundID != BSSoundHandle::kInvalidID) {
						xlSprint.Play();
					}
					if (xxlFootstepL.soundID != BSSoundHandle::kInvalidID) {
						xxlFootstepL.Play();
					}
				}
			}
		}
	}
}
