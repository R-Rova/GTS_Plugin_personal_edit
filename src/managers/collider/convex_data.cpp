#include "managers/collider/convex_data.hpp"
#include "hooks/RE.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {
	ConvexData::ConvexData(hkpConvexVerticesShape* shape) {
		shape->AddReference();
		this->convex = shape;

		this->radius = shape->radius;
		this->aabbHalfExtents = shape->aabbHalfExtents;
		this->aabbCenter = shape->aabbCenter;

		std::size_t numVertices = shape->numVertices;
		std::size_t numRotatedVerticies = numVertices / 3;
		if (numVertices % 3 != 0) {
			numRotatedVerticies += 1;
		}

		this->rotated_verts = {};
		for (std::size_t i = 0; i< numRotatedVerticies; i++) {
			this->rotated_verts.push_back(shape->rotatedVertices[i].vertices[0]);
			this->rotated_verts.push_back(shape->rotatedVertices[i].vertices[1]);
			this->rotated_verts.push_back(shape->rotatedVertices[i].vertices[2]);
		}
	}

	ConvexData::~ConvexData () {
		this->convex->RemoveReference();
	}

	void ConvexData::ApplyScale(const float& scale, const hkVector4& vecScale) {
		auto shape = this->convex;
		shape->radius = this->radius * scale;
		shape->aabbHalfExtents = this->aabbHalfExtents * vecScale;
		shape->aabbCenter = this->aabbCenter * vecScale;

		std::size_t numRotatedVerticies = this->rotated_verts.size();
		for (std::size_t i = 0; i < numRotatedVerticies; i++) {
			std::size_t j = i / 3;
			std::size_t k = i % 3;
			shape->rotatedVertices[j].vertices[k] = this->rotated_verts[i] * vecScale;
		}
	}
}
