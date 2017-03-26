#pragma once

#include "PhysicsEngine.h"
#include <iostream>
#include <iomanip>

namespace PhysicsEngine
{
	class PinballEncSimple : public StaticActor
	{
	public:
		PinballEncSimple(const PxTransform& pose = PxTransform(PxIdentity), PxVec3 dimensions = PxVec3(1.f, 1.f, 1.f), PxReal thickness = 1.f)
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
			GetShape(4)->setLocalPose(PxTransform(PxVec3(0, 0, -dimensions.z * 2)));

			// Inside walls in enclosure
			// Right 'plunger' wall
			CreateShape(PxBoxGeometry(PxVec3(thickness, (dimensions.y - thickness) / 1.75f, dimensions.z)));
			GetShape(5)->setLocalPose(PxTransform(PxVec3(dimensions.x - (dimensions.x / 10), -dimensions.y / 2.35f, 0)));
		}
	};

	class PinballEnclosure
	{
	private:
		PinballEncSimple *_simpEnclosure;
		ConvexMesh<StaticActor> *_topRightEntry;

	public:
		PinballEnclosure(const PxTransform& pose = PxTransform(PxIdentity), PxVec3 dimensions = PxVec3(1.f, 1.f, 1.f), PxReal thickness = 1.f)
		{
			// create the simple stuff
			_simpEnclosure = new PinballEncSimple(pose, dimensions, thickness);

			PxReal wallHeight = dimensions.z;

			dimensions /= 2;

			// Add top right entry thing
			std::vector<PxVec3> topRightVerts {
				//Start
				PxVec3(dimensions.x - (dimensions.x / 10) - thickness, dimensions.y / 7.2f, 0),
				PxVec3(dimensions.x - (dimensions.x / 10) + thickness, dimensions.y / 7.2f, 0),
				PxVec3(dimensions.x / 4, dimensions.y - (dimensions.y / 10), 0),
				PxVec3((dimensions.x / 4) + thickness * 2, dimensions.y - (dimensions.y / 10), 0),

				// End
				PxVec3(dimensions.x - (dimensions.x / 10) - thickness, dimensions.y / 7.2f, -1),
				PxVec3(dimensions.x - (dimensions.x / 10) + thickness, dimensions.y / 7.2f, -1),
				PxVec3(dimensions.x / 4, dimensions.y - (dimensions.y / 10), -1),
				PxVec3((dimensions.x / 4) + thickness * 2, dimensions.y - (dimensions.y / 10), -1),
			};

			_topRightEntry = new ConvexMesh<StaticActor>(topRightVerts, pose);
		}

		~PinballEnclosure()
		{
			delete _simpEnclosure;
			delete _topRightEntry;
		}

		void Color(PxVec3 new_color, PxU32 shape_index)
		{
			_simpEnclosure->Color(new_color, shape_index);
		}

		void AddToScene(Scene* scene)
		{
			scene->Add(_simpEnclosure);
			scene->Add(_topRightEntry);
		}
	};
}