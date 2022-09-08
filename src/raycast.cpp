#include "raycast.hpp"
#include "util.hpp"

using namespace Gts;
using namespace RE;

namespace Gts {

	void RayCollector::add_filter(NiObject* obj) noexcept {
		object_filter.push_back(obj);
	}
	bool RayCollector::is_filtered(NiObject* obj) {
		for (auto object: this->object_filter) {
			if (obj == object) {
				return true;
			}
		}
		return false;
	}
	bool RayCollector::is_filtered_av(NiAVObject* obj) {
		while (obj) {
			if (!is_filtered(obj)) {
				obj = obj->parent;
			} else {
				return true;
			}
		}
		return false;
	}

	void RayCollector::AddRayHit(const hkpCdBody& a_body, const hkpShapeRayCastCollectorOutput& a_hitInfo) {
		const hkpShape* shape; // = a_body.GetShape(); // Shape that was collided with

		// Search for top level shape
		const hkpCdBody* top_body = &a_body;
		while (top_body) {
			auto newShape = top_body->GetShape();
			if (newShape) {
				shape = newShape;
			}
			auto parent = top_body->parent;
			if (parent) {
				top_body = parent;
			} else {
				break;
			}
		}

		if (shape) {
			auto ni_shape = shape->userData;
			if (ni_shape) {
				if (is_filtered(ni_shape)) {
					return;
				}

				auto collision_node = ni_shape->AsBhkNiCollisionObject();
				if (collision_node) {
					auto av_node = collision_node->sceneObject;
					if (av_node) {
						if (is_filtered_av(av_node)) {
							return;
						}
					}
				}

				auto ni_node = ni_shape->AsNode();
				if (ni_node) {
					if (is_filtered_av(ni_node)) {
						return;
					}
				}
			}

			HitResult hit_result;
			hit_result.shape = shape;
			hit_result.fraction = a_hitInfo.hitFraction;
			hit_result.motion = a_body.motion;
			hit_result.rootCollidable = static_cast<const hkpCollidable*>(top_body);
			results.push_back(hit_result);
		}
	}

	std::vector<HitResult> CastRayResults(TESObjectCELL* cell, NiPoint3 in_origin, NiPoint3 direction, float unit_length, bool& success, const std::vector<NiAVObject*>& filterNodes) {
		float length = unit_to_meter(unit_length);
		success = false;
		if (!cell) {
			return {};
		}
		auto collision_world = cell->GetbhkWorld();
		if (!collision_world) {
			return {};
		}
		bhkPickData pick_data;

		NiPoint3 origin = unit_to_meter(in_origin);
		pick_data.rayInput.from = origin;

		NiPoint3 normed = direction / direction.Length();
		NiPoint3 end = origin + normed * length;
		pick_data.rayInput.to = end;

		NiPoint3 delta = end - origin;
		pick_data.ray = delta; // Length in each axis to travel

		RayCollector collector = RayCollector();
		for (auto filterNode: filterNodes) {
			collector.add_filter(filterNode);
		}
		// pick_data.rayHitCollectorA0 = &collector;
		pick_data.rayHitCollectorA8 = &collector;
		// pick_data.rayHitCollectorB0 = &collector;
		// pick_data.rayHitCollectorB8 = &collector;

		collision_world->PickObject(pick_data);
		success = !collector.results.empty();
		if (collector.results.size() > 0) {
			success = true;
			return collector.results;
		} else {
			return {};
		}
	}

	NiPoint3 CastRay(Actor* actor, NiPoint3 in_origin, NiPoint3 direction, float unit_length, bool& success) {
		float length = unit_to_meter(unit_length);
		success = false;
		if (!actor) {
			return NiPoint3();
		}
		auto cell = actor->GetParentCell();
		auto results = CastRayResults(cell, in_origin, direction, unit_length, success, { actor->Get3D(false), actor->Get3D(true)});

		float min_fraction = 1.0;
		NiPoint3 origin = unit_to_meter(in_origin);
		NiPoint3 normed = direction / direction.Length();
		if (results.size() > 0) {
			success = true;
			for (auto ray_result: results) {
				if (ray_result.fraction < min_fraction) {
					min_fraction = ray_result.fraction;
				}
			}
			return meter_to_unit(origin + normed * length * min_fraction);
		} else {
			return NiPoint3();
		}
	}
}


void hkpClosestRayHitCollector::AddRayHit(const hkpCdBody& a_body, const hkpShapeRayCastCollectorOutput& a_hitInfo) {
	// Dummy
}

hkpClosestRayHitCollector::~hkpClosestRayHitCollector() {
	// Dummy
}
