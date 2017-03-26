#pragma once

#include "PhysicsEngine.h"
#include <iostream>
#include <iomanip>

namespace PhysicsEngine
{
	class PinballEnclosure : public StaticActor
	{
	public:
		PinballEnclosure(const PxTransform& pose = PxTransform(PxIdentity), PxVec3 dimensions = PxVec3(1.f, 1.f, 1.f), PxReal thickness = 1.f)
			: StaticActor(pose)
		{
			dimensions /= 2;

			// create each side of the box
			CreateShape(PxBoxGeometry(PxVec3(dimensions.x + thickness, thickness, dimensions.z)), 1.f); // top
			CreateShape(PxBoxGeometry(PxVec3(dimensions.x + thickness, thickness, dimensions.z)), 1.f); // bottom
			CreateShape(PxBoxGeometry(PxVec3(thickness, dimensions.y - thickness, dimensions.z)), 1.f); // left
			CreateShape(PxBoxGeometry(PxVec3(thickness, dimensions.y - thickness, dimensions.z)), 1.f); // right

			// set the positions of the sides
			GetShape(0)->setLocalPose(PxTransform(PxVec3(0, dimensions.y, 0))); // top
			GetShape(1)->setLocalPose(PxTransform(PxVec3(0, -dimensions.y, 0))); // bottom
			GetShape(2)->setLocalPose(PxTransform(PxVec3(-dimensions.x, 0, 0))); // left
			GetShape(3)->setLocalPose(PxTransform(PxVec3(dimensions.x, 0, 0))); // right

			// create the bottom of the enclosure
			CreateShape(PxBoxGeometry(PxVec3(dimensions.x + thickness, dimensions.y + thickness, dimensions.z)), 1.f);
			GetShape(4)->setLocalPose(PxTransform(PxVec3(0, 0, -dimensions.z*2)));

			// Inside walls in enclosure
			// Right 'plunger' wall
			CreateShape(PxBoxGeometry(PxVec3(thickness, dimensions.y - thickness, dimensions.z)));
			GetShape(5)->setLocalPose(PxTransform(PxVec3(-dimensions.x, 0, 0)));
		}
	};
}