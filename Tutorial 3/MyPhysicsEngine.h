#pragma once

#include "BasicActors.h"
#include "ComplexActors.h"
#include <iostream>
#include <iomanip>
#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */

namespace PhysicsEngine
{
	using namespace std;

	//a list of colours: Circus Palette
	static const PxVec3 color_palette[] = {PxVec3(46.f/255.f,9.f/255.f,39.f/255.f),PxVec3(217.f/255.f,0.f/255.f,0.f/255.f),
		PxVec3(255.f/255.f,45.f/255.f,0.f/255.f),PxVec3(255.f/255.f,140.f/255.f,54.f/255.f),PxVec3(4.f/255.f,117.f/255.f,111.f/255.f)};

	struct FilterGroup
	{
		enum Enum
		{
			BALL		= (1 << 0),
			PADDLES		= (1 << 1),
			HEXAGONS	= (1 << 2)
			//add more if you need
		};
	};

	///A customised collision class, implemneting various callbacks
	class MySimulationEventCallback : public PxSimulationEventCallback
	{
	public:
		bool onTouch = false;
		bool resetTrigger = false;
		bool scoreTrigger = false;
		bool additionalScoreAllowed = false;

		MySimulationEventCallback(){}

		///Method called when the contact with the trigger object is detected.
		virtual void onTrigger(PxTriggerPair* pairs, PxU32 count) 
		{
			//you can read the trigger information here
			for (PxU32 i = 0; i < count; i++)
			{
				//filter out contact with the planes
				if (pairs[i].otherShape->getGeometryType() != PxGeometryType::ePLANE)
				{
					//check if eNOTIFY_TOUCH_FOUND trigger
					if (pairs[i].status & PxPairFlag::eNOTIFY_TOUCH_FOUND)
					{
						//cerr << "onTrigger::eNOTIFY_TOUCH_FOUND" << endl;
						string triggerActorName = std::string(pairs[i].triggerActor->getName());
						string otherActorName = std::string(pairs[i].otherActor->getName());

						if (triggerActorName == "scoreTrigger" && otherActorName == "Ball")
						{
							if (additionalScoreAllowed)
							{
								scoreTrigger = true;
								additionalScoreAllowed = false;
							}
						}

						if (triggerActorName == "deathTrigger")
						{
							resetTrigger = true;
						}
					}
					//check if eNOTIFY_TOUCH_LOST trigger
					if (pairs[i].status & PxPairFlag::eNOTIFY_TOUCH_LOST)
					{
						//cerr << "onTrigger::eNOTIFY_TOUCH_LOST" << endl;
						if (resetTrigger) !resetTrigger;
					}
				}
			}
		}

		///Method called when the contact by the filter shader is detected.
		virtual void onContact(const PxContactPairHeader &pairHeader, const PxContactPair *pairs, PxU32 nbPairs) 
		{
			//cerr << "Contact found between " << pairHeader.actors[0]->getName() << " " << pairHeader.actors[1]->getName() << endl;

			//check all pairs
			for (PxU32 i = 0; i < nbPairs; i++)
			{
				//check eNOTIFY_TOUCH_FOUND
				if (pairs[i].events & PxPairFlag::eNOTIFY_TOUCH_FOUND)
				{
					//cerr << "onContact::eNOTIFY_TOUCH_FOUND" << endl;
					if (pairHeader.actors[0]->getName() == "Paddle" || pairHeader.actors[1]->getName() == "Paddle")
						additionalScoreAllowed = true;
					if (pairHeader.actors[0]->getName() == "Hexagon" || pairHeader.actors[1]->getName() == "Hexagon")
						additionalScoreAllowed = false;
				}
				//check eNOTIFY_TOUCH_LOST
				if (pairs[i].events & PxPairFlag::eNOTIFY_TOUCH_LOST)
				{
					//cerr << "onContact::eNOTIFY_TOUCH_LOST" << endl;
				}
			}
		}

		virtual void onConstraintBreak(PxConstraintInfo *constraints, PxU32 count) {}
		virtual void onWake(PxActor **actors, PxU32 count) {}
		virtual void onSleep(PxActor **actors, PxU32 count) {}
	};

	//A simple filter shader based on PxDefaultSimulationFilterShader - without group filtering
	static PxFilterFlags CustomFilterShader( PxFilterObjectAttributes attributes0,	PxFilterData filterData0,
		PxFilterObjectAttributes attributes1,	PxFilterData filterData1,
		PxPairFlags& pairFlags,	const void* constantBlock,	PxU32 constantBlockSize)
	{
		// let triggers through
		if(PxFilterObjectIsTrigger(attributes0) || PxFilterObjectIsTrigger(attributes1))
		{
			pairFlags = PxPairFlag::eTRIGGER_DEFAULT;
			return PxFilterFlags();
		}

		pairFlags = PxPairFlag::eCONTACT_DEFAULT;
		//enable continous collision detection
		pairFlags |= PxPairFlag::eCCD_LINEAR;
		
		
		//customise collision filtering here
		//e.g.

		// trigger the contact callback for pairs (A,B) where 
		// the filtermask of A contains the ID of B and vice versa.
		if((filterData0.word0 & filterData1.word1) && (filterData1.word0 & filterData0.word1))
		{
			//trigger onContact callback for this pair of objects
			pairFlags |= PxPairFlag::eNOTIFY_TOUCH_FOUND;
			pairFlags |= PxPairFlag::eNOTIFY_TOUCH_LOST;
//			pairFlags |= PxPairFlag::eNOTIFY_CONTACT_POINTS;
		}

		return PxFilterFlags();
	};

	///Custom scene class
	class MyScene : public Scene
	{
		Plane* plane;
		MySimulationEventCallback* my_callback;

		PinballEnclosure* enclosure;

		Paddle* paddles[2];

		Trampoline* plunger;

		PinballBall* ball;
		
		PxMaterial* ballMaterial;
		PxMaterial* woodMaterial;

		Hexagon* hexagons[3];

		Box<StaticActor>* scoreTrigger;

		int score = 0;
		
	public:
		//specify your custom filter shader here
		//PxDefaultSimulationFilterShader by default
		MyScene() : Scene(CustomFilterShader) {};

		///A custom scene class
		void SetVisualisation()
		{
			px_scene->setVisualizationParameter(PxVisualizationParameter::eSCALE, 1.0f);
			px_scene->setVisualizationParameter(PxVisualizationParameter::eCOLLISION_SHAPES, 1.0f);
			px_scene->setVisualizationParameter(PxVisualizationParameter::eBODY_LIN_VELOCITY, 1.0f);
			px_scene->setVisualizationParameter(PxVisualizationParameter::eBODY_AXES, 1.0f);
			px_scene->setVisualizationParameter(PxVisualizationParameter::eJOINT_LOCAL_FRAMES, 1.0f);
			px_scene->setVisualizationParameter(PxVisualizationParameter::eJOINT_LIMITS, 1.0f);
		}

		//Custom scene initialisation
		virtual void CustomInit()
		{
			srand(time(NULL));

			SetVisualisation();

			GetMaterial()->setDynamicFriction(.2f);
			GetMaterial()->setStaticFriction(0.2f);

			// Ball Material - see http://www.tribology-abc.com/abc/cof.htm & http://hypertextbook.com/facts/2006/restitution.shtml
			ballMaterial = CreateMaterial(0.5f, 0.2f, 0.597);
			woodMaterial = CreateMaterial(0.0f, 0.0f, 0.597);

			///Initialise and set the customised event callback
			my_callback = new MySimulationEventCallback();
			px_scene->setSimulationEventCallback(my_callback);

			plane = new Plane();
			plane->Color(PxVec3(210.f / 255.f, 210.f / 255.f, 210.f / 255.f));
			Add(plane);

			// Pinball Enclosure --------------------------
			PxVec3 encDims = PxVec3(10.f, 20.f, .5f);
			PxTransform encPose = PxTransform(PxVec3(0.f, 6.2f, -2.8f), PxQuat(-PxPi / 3, PxVec3(1.f, 0.f, 0.f)));
			enclosure = new PinballEnclosure(encPose, encDims, .1f);
			enclosure->Color(color_palette[2], 4);
			enclosure->SetMaterial(woodMaterial);
			enclosure->AddToScene(this);

			// Paddles ------------------------------------
			// Left
			paddles[0] = new Paddle(PxTransform(encPose.p + PxVec3(-2.08f, -3.75f, 6.58f), encPose.q), PxVec3(1.5f, .3f, .5f), 0.1f);
			paddles[0]->Material(woodMaterial);
			paddles[0]->SetupFiltering(FilterGroup::PADDLES, FilterGroup::BALL);
			Add(paddles[0]);

			// Right
			paddles[1] = new Paddle(PxTransform(encPose.p + PxVec3(1.53f, -3.75f, 6.58f), encPose.q * PxQuat(PxPi, PxVec3(0, 1, 0))), PxVec3(1.5f, .3f, .5f), 0.1f);
			paddles[1]->Material(woodMaterial);
			paddles[1]->SetupFiltering(FilterGroup::PADDLES, FilterGroup::BALL);
			Add(paddles[1]);

			// Plunger ------------------------------------
			plunger = new Trampoline(PxTransform(encPose.p + PxVec3(4.75f, -5.1f, 8.4f), encPose.q), PxVec3(0.1f, 0.25f, 0.1f), 100.f, 25.f);
			plunger->AddToScene(this);

			// Ball ---------------------------------------
			CreateBall(PxTransform(encPose.p + PxVec3(4.75f, -3.3f, 6.4f)));

			// Hexagons -----------------------------------
			hexagons[0] = new Hexagon(encPose, 1.f, encDims.z);
			hexagons[1] = new Hexagon(PxTransform(encPose.p + PxVec3(-2.7f, -1.65f, 2.8f), encPose.q), 1.f, encDims.z);
			hexagons[2] = new Hexagon(PxTransform(encPose.p + PxVec3(2.7f, -1.65f, 2.8f), encPose.q), 1.f, encDims.z);
			hexagons[0]->SetupFiltering(FilterGroup::HEXAGONS, FilterGroup::BALL);
			hexagons[1]->SetupFiltering(FilterGroup::HEXAGONS, FilterGroup::BALL);
			hexagons[2]->SetupFiltering(FilterGroup::HEXAGONS, FilterGroup::BALL);
			((PxRigidActor*)hexagons[0]->Get())->setName("Hexagon");
			((PxRigidActor*)hexagons[1]->Get())->setName("Hexagon");
			((PxRigidActor*)hexagons[2]->Get())->setName("Hexagon");
			Add(hexagons[0]);
			Add(hexagons[1]);
			Add(hexagons[2]);

			// Score Trigger
			scoreTrigger = new Box<StaticActor>(encPose, PxVec3(encDims.x/2, encDims.y / 15, encDims.z));
			((PxRigidActor*)scoreTrigger->Get())->setGlobalPose(PxTransform(encPose.p + PxVec3(0, 1.5f, -2.f), encPose.q));
			((PxRigidActor*)scoreTrigger->Get())->setActorFlag(PxActorFlag::eVISUALIZATION, false);
			((PxRigidActor*)scoreTrigger->Get())->setName("scoreTrigger");
			scoreTrigger->SetTrigger(true);
			Add(scoreTrigger);
			
		}

		//Custom udpate function
		virtual void CustomUpdate() 
		{
			// check if the game needs to be reset
			if (my_callback->resetTrigger)
				ResetGame();
			if (my_callback->scoreTrigger)
			{
				my_callback->scoreTrigger = false;
				cout << "Extra Score" << endl;
			}
		}

		void AddFlipperForce(int flipperID, PxReal force)
		{
			paddles[flipperID]->GetJoint()->DriveVelocity(force);
		}

		void AddPlungerForce()
		{
			plunger->AddPlungerForce(20.f);
		}

		void CreateBall(const PxTransform& pose)
		{
			my_callback->additionalScoreAllowed = false;
			if (ball != nullptr)
			{
				px_scene->removeActor(*(PxActor*)ball->Get());
				delete ball;
			}
			ball = new PinballBall(pose, 0.15f, 0.01f);
			((PxRigidBody*)ball->Get())->setRigidBodyFlag(PxRigidBodyFlag::eENABLE_CCD, true);
			ball->Material(ballMaterial);
			ball->SetupFiltering(FilterGroup::BALL, FilterGroup::PADDLES | FilterGroup::HEXAGONS);
			((PxRigidActor*)ball->Get())->setName("Ball");
			Add(ball);
		}

		void ResetGame()
		{
			my_callback->resetTrigger = false;
			PxTransform ballPose(ball->initalPose);
			CreateBall(ballPose);
		}
	};
}
