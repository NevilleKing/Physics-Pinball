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

			// Add bottom walls next to bumpers
			CreateShape(PxBoxGeometry(PxVec3(dimensions.x / 3, thickness, dimensions.z)));
			GetShape(6)->setLocalPose(PxTransform(PxVec3(-dimensions.x + (dimensions.x / 3.55f), -dimensions.y + (dimensions.y / 3), 0), PxQuat(-PxPi / 6, PxVec3(0, 0, 1))));

			CreateShape(PxBoxGeometry(PxVec3(dimensions.x / 3, thickness, dimensions.z)));
			GetShape(7)->setLocalPose(PxTransform(PxVec3(dimensions.x - (dimensions.x / 2.55f), -dimensions.y + (dimensions.y / 3), 0), PxQuat(PxPi / 6, PxVec3(0, 0, 1))));
		}
	};

	class PinballEnclosure
	{
	private:
		PinballEncSimple *_simpEnclosure;
		ConvexMesh<StaticActor> *_topRightEntry;

		PxVec3 _dimensions;

		Box<StaticActor>* _deathTrigger;

	public:
		PinballEnclosure(const PxTransform& pose = PxTransform(PxIdentity), PxVec3 dimensions = PxVec3(1.f, 1.f, 1.f), PxReal thickness = 1.f)
		{
			_dimensions = dimensions;

			// create the simple stuff
			_simpEnclosure = new PinballEncSimple(pose, dimensions, thickness);

			PxReal wallHeight = dimensions.z;

			dimensions /= 2;

			// Add top right entry thing
			std::vector<PxVec3> topRightVerts {
				PxVec3(dimensions.x - thickness, dimensions.y - thickness, dimensions.z), // top left
				PxVec3(dimensions.x - thickness, (dimensions.y - thickness) - (dimensions.y / 6), dimensions.z), // right
				PxVec3((dimensions.x - thickness) - (dimensions.x / 4), dimensions.y - thickness, dimensions.z), // top
				PxVec3(dimensions.x - thickness, dimensions.y - thickness, -dimensions.z), // top left
				PxVec3(dimensions.x - thickness, (dimensions.y - thickness) - (dimensions.y / 6), -dimensions.z), // right
				PxVec3((dimensions.x - thickness) - (dimensions.x / 4), dimensions.y - thickness, -dimensions.z) // top
			};

			_topRightEntry = new ConvexMesh<StaticActor>(vector<PxVec3>(begin(topRightVerts), end(topRightVerts)), pose);

			// Add reset trigger at bottom of machine
			PxVec3 deathDims(dimensions.x - (dimensions.x / 17), dimensions.y / 20, dimensions.z);
			_deathTrigger = new Box<StaticActor>(PxTransform(pose.p + PxVec3(-(dimensions.x / 17), -dimensions.y / 2.1f, dimensions.z*32.5f), pose.q), deathDims);
			_deathTrigger->SetTrigger(true);
			((PxActor*)_deathTrigger->Get())->setActorFlag(PxActorFlag::eVISUALIZATION, false);
		}

		~PinballEnclosure()
		{
			delete _simpEnclosure;
			delete _topRightEntry;
			delete _deathTrigger;
		}

		void Color(PxVec3 new_color, PxU32 shape_index)
		{
			_simpEnclosure->Color(new_color, shape_index);
		}

		void AddToScene(Scene* scene)
		{
			scene->Add(_simpEnclosure);
			scene->Add(_topRightEntry);
			scene->Add(_deathTrigger);
		}

		PxVec3 GetDimensions()
		{
			return _dimensions;
		}

		void SetMaterial(PxMaterial *material)
		{
			_simpEnclosure->Material(material);
			_topRightEntry->Material(material);
		}
	};

	///An example class showing the use of springs (distance joints).
	class Trampoline
	{
		vector<DistanceJoint*> springs;
		Box<DynamicActor> *top;
		Box<StaticActor> *bottom;

	public:
		Trampoline(const PxTransform& pose = PxTransform(PxIdentity), const PxVec3& dimensions = PxVec3(1.f, 1.f, 1.f), PxReal stiffness = 1.f, PxReal damping = 1.f)
		{
			PxReal thickness = .1f;
			bottom = new Box<StaticActor>(PxTransform(PxVec3(pose.p.x, pose.p.y + thickness, pose.p.z), pose.q), PxVec3(dimensions.x, thickness, dimensions.z));
			top = new Box<DynamicActor>(PxTransform(PxVec3(pose.p.x, pose.p.y + (dimensions.y + thickness), pose.p.z), pose.q), PxVec3(dimensions.x, thickness, dimensions.z));
			springs.resize(4);
			springs[0] = new DistanceJoint(bottom, PxTransform(PxVec3(dimensions.x, thickness, dimensions.z)), top, PxTransform(PxVec3(dimensions.x, -dimensions.y, dimensions.z)));
			springs[1] = new DistanceJoint(bottom, PxTransform(PxVec3(dimensions.x, thickness, -dimensions.z)), top, PxTransform(PxVec3(dimensions.x, -dimensions.y, -dimensions.z)));
			springs[2] = new DistanceJoint(bottom, PxTransform(PxVec3(-dimensions.x, thickness, dimensions.z)), top, PxTransform(PxVec3(-dimensions.x, -dimensions.y, dimensions.z)));
			springs[3] = new DistanceJoint(bottom, PxTransform(PxVec3(-dimensions.x, thickness, -dimensions.z)), top, PxTransform(PxVec3(-dimensions.x, -dimensions.y, -dimensions.z)));

			for (unsigned int i = 0; i < springs.size(); i++)
			{
				springs[i]->Stiffness(stiffness);
				springs[i]->Damping(damping);
			}
		}

		void AddToScene(Scene* scene)
		{
			scene->Add(bottom);
			scene->Add(top);
		}

		~Trampoline()
		{
			for (unsigned int i = 0; i < springs.size(); i++)
				delete springs[i];
		}

		void AddPlungerForce(PxReal force)
		{
			((PxRigidDynamic*)top->Get())->addForce(PxVec3(0, force/2, -force));
		}
	};

	class Paddle : public ConvexMesh<DynamicActor>
	{
	private:
		RevoluteJoint* joint;

	public:
		Paddle(PxTransform pose = PxTransform(PxIdentity), PxVec3 dimensions = PxVec3(2.0f, 1.0f, 1.0f), PxReal density = 1.f)
			:ConvexMesh<DynamicActor>(pose, density)
		{
			PxVec3 wedge_verts[] = { PxVec3(dimensions.x, 0.f, -dimensions.z/2), PxVec3(0.f, -dimensions.y/2, -dimensions.z / 2), PxVec3(0.f, dimensions.y/2, -dimensions.z / 2),
									 PxVec3(dimensions.x, 0.f, dimensions.z / 2), PxVec3(0.f, -dimensions.y/2, dimensions.z / 2), PxVec3(0.f, dimensions.y/2, dimensions.z / 2) };

			ConvexMesh<DynamicActor>::AddVerts(vector<PxVec3>(std::begin(wedge_verts), end(wedge_verts)));

			joint = new RevoluteJoint(nullptr, PxTransform(pose.p, pose.q * PxQuat(-PxPi / 2, PxVec3(0, 1, 0))), this, PxTransform(PxVec3(0,0,0), PxQuat(-PxPi / 2, PxVec3(0, 1, 0))));
			joint->SetLimits(-PxPi / 6, PxPi / 6);
		}

		~Paddle()
		{
			if (joint != nullptr)
				delete joint;
		}

		RevoluteJoint* GetJoint()
		{
			return joint;
		}
	};
}