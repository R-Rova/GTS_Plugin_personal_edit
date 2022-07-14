#include "highheel.h"
#include "node.h"
#include "scale.h"
#include "GtsManager.h"
#include "persistent.h"
#include "transient.h"
#include "util.h"

using namespace RE;
using namespace Gts;

namespace {
	NiAVObject* find_any_node(Actor* actor, std::string_view name) {
		for (bool person: {false, true}) {
			auto found = find_node(actor, name, person);
			if (found) {
				return found;
			}
		}
		return nullptr;
	}
}

namespace Gts {
	void apply_high_heel_scale(Actor* actor, TempActorData* temp_data) {
		if (!actor) {
			return;
		}
		if (!temp_data) {
			return;
		}
		if (logit(actor)) log::info("High heels for: {}", actor_name(actor));


		float new_hh = 0.0;
		float last_hh_adjustment = temp_data->last_hh_adjustment;
		if (!Persistent::GetSingleton().highheel_correction) {
			if (fabs(last_hh_adjustment) > 1e-5) {
				log::info("Last hh adjustment to turn it off");
			} else {
				return;
			}
		} else {
			NiAVObject* npc_node = find_any_node(actor, "NPC");
			if (!npc_node) {
				if (logit(actor)) log::info("NPC node missing");
				return;
			}

			NiAVObject* root_node = find_any_node(actor, "NPC Root [Root]");
			if (!root_node) {
				if (logit(actor)) log::info("NPC Root node missing");
				return;
			}

			NiAVObject* com_node = find_any_node(actor, "NPC COM [COM ]");
			if (!com_node) {
				if (logit(actor)) log::info("NPC COM node missing");
				return;
			}

			NiAVObject* body_node = find_any_node(actor, "CME Body [Body]");
			if (!body_node) {
				if (logit(actor)) log::info("CME Body node missing");
				return;
			}

			float base_hh = npc_node->local.translate.z;
			if (logit(actor)) log::info("Base HH: {}", base_hh);
			float scale = root_node->local.scale;
			log::info("NPC Root Scale: {}", scale);
			new_hh = (scale * base_hh - base_hh) / (com_node->local.scale * root_node->local.scale * npc_node->local.scale);
			if (logit(actor)) log::info("CME Body.z = {}", new_hh);
		}

		bool adjusted = false;
		for (bool person: {false, true}) {
			auto npc_root_node = find_node(actor, "CME Body [Body]", person);
			if (npc_root_node) {
                float current_value = npc_root_node->local.translate.z;
                if ((fabs(last_hh_adjustment - new_hh) > 1e-5) || (fabs(current_value - new_hh) > 1e-5)) {
					npc_root_node->local.translate.z = new_hh;
					NiUpdateData ctx;
					npc_root_node->UpdateWorldData(&ctx);
					if (logit(actor)) log::info("CME updated");
                    adjusted = true;
                }
			} else {
				if (logit(actor)) log::info("Cannot set node: CME Body: First Person: {}", person);
			}
		}
		if (adjusted) {
			temp_data->last_hh_adjustment = new_hh;
		}
	}
}
