#include "managers/altcamera.hpp"
#include "scale/scale.hpp"
#include "util.hpp"
#include "data/runtime.hpp"
#include "data/persistent.hpp"
#include "Config.hpp"
#include "node.hpp"

using namespace SKSE;
using namespace RE;
using namespace REL;
using namespace Gts;

namespace {
	void SetINIFloat(std::string_view name, float value) {
		auto ini_conf = INISettingCollection::GetSingleton();
		Setting* setting = ini_conf->GetSetting(name);
		if (setting) {
			setting->data.f=value; // If float
			ini_conf->WriteSetting(setting);
		}
	}

	void UpdateThirdPerson() {
		auto camera = PlayerCamera::GetSingleton();
		auto player = PlayerCharacter::GetSingleton();
		if (camera && player) {
			camera->UpdateThirdPerson(player->IsWeaponDrawn());
		}
	}

	void ResetIniSettings() {
		SetINIFloat("fOverShoulderPosX:Camera", 30.0);
		SetINIFloat("fOverShoulderPosY:Camera", 30.0);
		SetINIFloat("fOverShoulderPosZ:Camera", -10.0);
		SetINIFloat("fOverShoulderCombatPosX:Camera", 0.0);
		SetINIFloat("fOverShoulderCombatPosY:Camera", 0.0);
		SetINIFloat("fOverShoulderCombatPosZ:Camera", 20.0);
		SetINIFloat("fVanityModeMaxDist:Camera", 600.0);
		SetINIFloat("fVanityModeMinDist:Camera", 155.0);
		SetINIFloat("fMouseWheelZoomSpeed:Camera", 0.8000000119);
		SetINIFloat("fMouseWheelZoomIncrement:Camera", 0.075000003);
		UpdateThirdPerson();
	}

	NiCamera* GetNiCamera() {
		auto camera = PlayerCamera::GetSingleton();
		auto cameraRoot = camera->cameraRoot.get();
		NiCamera* niCamera = nullptr;
		for (auto child: cameraRoot->GetChildren()) {
			NiAVObject* node = child.get();
			if (node) {
				NiCamera* casted = netimmerse_cast<NiCamera*>(node);
				if (casted) {
					niCamera = casted;
					break;
				}
			}
		}
		return niCamera;
	}
	void UpdateWorld2ScreetMat(NiCamera* niCamera) {
		auto camNi = niCamera ? niCamera : GetNiCamera();
		typedef void (*UpdateWorldToScreenMtx)(RE::NiCamera*);
		static auto toScreenFunc = REL::Relocation<UpdateWorldToScreenMtx>(REL::RelocationID(69271, 70641).address());
		toScreenFunc(camNi);
	}
	ShadowSceneNode* GetShadowMap() {
		auto player = PlayerCharacter::GetSingleton();
		if (player) {
			auto searchRoot = player->GetCurrent3D();
			if (searchRoot) {
				NiNode* parent = searchRoot->parent;
				while (parent) {
					log::info("- {}", GetRawName(parent));
					log::info("- {}", parent->name);
					ShadowSceneNode* shadowNode = skyrim_cast<ShadowSceneNode*>(parent);
					if (shadowNode) {
						return shadowNode;
					}
					parent = parent->parent;
				}
			}
		}
		return nullptr;
	}

	void Experiment10() {
		auto camera = PlayerCamera::GetSingleton();
		auto cameraRoot = camera->cameraRoot.get();
		NiCamera* niCamera = nullptr;
		for (auto child: cameraRoot->GetChildren()) {
			NiAVObject* node = child.get();
			log::info("- {}", GetRawName(node));
			if (node) {
				NiCamera* casted = netimmerse_cast<NiCamera*>(node);
				if (casted) {
					niCamera = casted;
					break;
				}
			}
		}
		if (niCamera) {
			auto player = PlayerCharacter::GetSingleton();
			if (player) {
				float scale = get_visual_scale(player);
				if (scale > 1e-4) {
					auto model = player->Get3D(false);
					if (model) {
						auto playerTrans = model->world;
						auto playerTransInve = model->world.Invert();

						// Get Scaled Camera Location
						auto cameraLocation = cameraRoot->world.translate;
						auto targetLocationWorld = playerTrans*((playerTransInve*cameraLocation) * scale);
						auto parent = niCamera->parent;
						NiTransform transform = parent->world.Invert();
						auto targetLocationLocal = transform * targetLocationWorld;

						// Add adjustments
						// log::info("Delta: {},{}", deltaX, deltaY);
						// targetLocationLocal.x += deltaX * scale;
						// targetLocationLocal.y += deltaY * scale;

						// Set Camera
						niCamera->local.translate = targetLocationLocal;
						niCamera->world.translate = targetLocationLocal;
						update_node(niCamera);
						UpdateWorld2ScreetMat(nullptr);
					}
				}
			}
		}
	}

	void Experiment11() {
		auto camera = PlayerCamera::GetSingleton();
		auto third = skyrim_cast<ThirdPersonState*>(camera->cameraStates[CameraState::kThirdPerson].get());
		// log::info("Cam node pos: {}::{}", Vector2Str(third->thirdPersonCameraObj->world.translate), Vector2Str(third->thirdPersonCameraObj->local.translate));
		NiPoint3 thirdLocation;
		third->GetTranslation(thirdLocation);

		auto cameraRoot = camera->cameraRoot;

		auto player = PlayerCharacter::GetSingleton();
		if (player) {
			float scale = get_visual_scale(player);
			if (scale > 1e-4) {
				auto model = player->Get3D(false);
				if (model) {
					auto playerTrans = model->world;
					auto playerTransInve = model->world.Invert();

					// Get Scaled Camera Location
					auto cameraLocation = thirdLocation;
					log::info("cameraLocation: {}", Vector2Str(cameraLocation));
					auto targetLocationWorld = playerTrans*((playerTransInve*cameraLocation) * scale);
					auto parent = cameraRoot->parent;
					NiTransform transform = parent->world.Invert();
					auto targetLocationLocal = transform * targetLocationWorld;
					log::info("targetLocationLocal: {}", Vector2Str(targetLocationLocal));

					// Add adjustments
					// log::info("Delta: {},{}", deltaX, deltaY);
					// targetLocationLocal.x += deltaX * scale;
					// targetLocationLocal.y += deltaY * scale;

					// Set Camera
					cameraRoot->local.translate = targetLocationLocal;
					third->translation = targetLocationLocal;
					update_node(cameraRoot.get());
				}
			}
		}
	}

	void Experiment12() {
		auto camera = PlayerCamera::GetSingleton();
		auto player = PlayerCharacter::GetSingleton();
		if (player) {
			float scale = get_visual_scale(player);
			if (scale > 1e-4) {
				string node_name = "NPC Root [Root]";
				auto model = find_node(player, node_name, false);
				auto node = model->AsNode();
				static NiPointer<NiNode> nodePtr = NiPointer(node);
				if (node) {
					log::info("Can the root be changed.");
					camera->cameraRoot = nodePtr;
					log::info("Changed.");
				}
			}
		}
	}

	void Experiment13() {
		auto camera = PlayerCamera::GetSingleton();
		auto cameraRoot = camera->cameraRoot;
		auto player = PlayerCharacter::GetSingleton();
		auto currentState = camera->currentState;

		if (cameraRoot) {
			NiPoint3 cameraLocation;
			currentState->GetTranslation(cameraLocation);
			if (currentState) {
				if (player) {
					float scale = get_visual_scale(player);
					if (scale > 1e-4) {
						auto model = player->Get3D(false);
						if (model) {
							auto playerTrans = model->world;
							auto playerTransInve = model->world.Invert();

							// Get Scaled Camera Location
							auto targetLocationWorld = playerTrans*((playerTransInve*cameraLocation) * scale);
							auto parent = cameraRoot->parent;
							NiTransform transform = parent->world.Invert();
							auto targetLocationLocal = transform * targetLocationWorld;

							// Add adjustments
							// log::info("Delta: {},{}", deltaX, deltaY);
							// targetLocationLocal.x += deltaX * scale;
							// targetLocationLocal.y += deltaY * scale;

							// Set Camera
							cameraRoot->local.translate = targetLocationLocal;
							cameraRoot->world.translate = targetLocationLocal;
							update_node(cameraRoot.get());

							auto niCamera = GetNiCamera();
							if (niCamera) {
								log::info("NiUpdate");
								niCamera->world.translate = targetLocationLocal;
								UpdateWorld2ScreetMat(niCamera);
							}
							auto shadowNode = GetShadowMap();
							if (shadowNode) {
								log::info("Update shadow map");
								shadowNode->GetRuntimeData().cameraPos = targetLocationLocal;
							}
							log::info("Set EXP13");
						}
					}
				}
			}
		}
	}
}

namespace Gts {
	CameraManager& CameraManager::GetSingleton() noexcept {
		static CameraManager instance;
		return instance;
	}

	std::string CameraManager::DebugName() {
		return "CameraManager";
	}

	void CameraManager::Start() {
		ResetIniSettings();
		// Experiment12();
	}

	void CameraManager::Update() {
		// Experiment11();
		Experiment13();
	}
	void CameraManager::HavokUpdate() {
		// Experiment11();
		// Experiment13();
	}

	void CameraManager::AdjustUpDown(float amt) {
		this->deltaZ += amt;
	}
	void CameraManager::ResetUpDown() {
		this->deltaZ = 0.0;
	}

	void CameraManager::AdjustLeftRight(float amt) {
		this->deltaX += amt;
	}
	void CameraManager::ResetLeftRight() {
		this->deltaX = 0.0;
	}
}
