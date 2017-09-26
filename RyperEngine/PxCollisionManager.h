#pragma once

//PhysX
#include <PxPhysicsAPI.h>

namespace PX
{

class CollisionManager : physx::PxSimulationEventCallback
{
	physx::PxRigidDynamic*	mPlayer = nullptr;
	physx::PxRigidStatic*	mPickup = nullptr;
	physx::PxScene*			mScene	= nullptr;
public:
	struct FilterGroup
	{
		enum Enum
		{
			ePLAYER			= (1 << 0),
			ePICKUP			= (1 << 1),
		};
	};

	CollisionManager () {}
	void SetPlayer (physx::PxRigidDynamic* player)
	{
		mPlayer = player;
		setupFiltering(player, FilterGroup::ePLAYER, FilterGroup::ePICKUP);
	}
	void SetPickup(physx::PxRigidStatic* pickup)
	{
		mPickup = pickup;
		setupFiltering(pickup, FilterGroup::ePICKUP, FilterGroup::ePLAYER);
	}
	void SetScene(physx::PxScene *scene) {mScene = scene;}
	virtual void onContact(const physx::PxContactPairHeader& pairHeader, const physx::PxContactPair* pairs, physx::PxU32 nbPairs) override
	{
		//for (physx::PxU32 i = 0; i < nbPairs; i++)
		//{
		//	const physx::PxContactPair& cp = pairs[i];

		//	if (cp.events & physx::PxPairFlag::eNOTIFY_TOUCH_FOUND)
		//	{
		//		if ((pairHeader.actors[0] == mSubmarineActor) || (pairHeader.actors[1] == mSubmarineActor))
		//		{
		//			physx::PxActor* otherActor = (mSubmarineActor == pairHeader.actors[0]) ? pairHeader.actors[1] : pairHeader.actors[0];
		//			physx::Seamine* mine = reinterpret_cast<Seamine*>(otherActor->userData);
		//			// insert only once
		//			if (std::find(mMinesToExplode.begin(), mMinesToExplode.end(), mine) == mMinesToExplode.end())
		//				mMinesToExplode.push_back(mine);

		//			break;
		//		}
		//	}
		//}
	}

	//For pickup
	virtual void onTrigger(physx::PxTriggerPair* pairs, physx::PxU32 count) override
	{
		for (physx::PxU32 i = 0; i < count; i++)
		{
			// ignore pairs when shapes have been deleted
			if (pairs[i].flags & (physx::PxTriggerPairFlag::eREMOVED_SHAPE_TRIGGER | physx::PxTriggerPairFlag::eREMOVED_SHAPE_OTHER))
				continue;

			//TODO Check some kind of type safetiness
			if ((pairs[i].otherActor == mPlayer) && (pairs[i].triggerActor == mPickup))
			{
				std::cout << "pickup" << std::endl;
			}
		}
	}
	static void setupFiltering(physx::PxRigidActor* actor, physx::PxU32 filterGroup, physx::PxU32 filterMask)
	{
		physx::PxFilterData filterData;
		filterData.word0 = filterGroup; // word0 = own ID
		filterData.word1 = filterMask;  // word1 = ID mask to filter pairs that trigger a
										// contact callback;
		const physx::PxU32 numShapes = actor->getNbShapes();
		physx::PxShape** shapes = (physx::PxShape**)malloc(sizeof(physx::PxShape*)*numShapes);
		actor->getShapes(shapes, numShapes);
		for (physx::PxU32 i = 0; i < numShapes; i++)
		{
			physx::PxShape* shape = shapes[i];
			shape->setSimulationFilterData(filterData);
		}
		free(shapes);
	}
	virtual void	onConstraintBreak(PxConstraintInfo*, PxU32) override {}
	virtual void	onWake(PxActor**, PxU32) override {}
	virtual void	onSleep(PxActor**, PxU32) override {}
	virtual void	onAdvance(const PxRigidBody*const*, const PxTransform*, const PxU32) override {}

	static physx::PxFilterFlags SampleFilterShader(
		physx::PxFilterObjectAttributes attributes0, physx::PxFilterData filterData0,
		physx::PxFilterObjectAttributes attributes1, physx::PxFilterData filterData1,
		physx::PxPairFlags& pairFlags, const void* constantBlock, physx::PxU32 constantBlockSize)
	{
		// let triggers through
		if (physx::PxFilterObjectIsTrigger(attributes0) || physx::PxFilterObjectIsTrigger(attributes1))
		{
			pairFlags = physx::PxPairFlag::eTRIGGER_DEFAULT;
			return physx::PxFilterFlag::eDEFAULT;
		}
		// generate contacts for all that were not filtered above
		pairFlags = physx::PxPairFlag::eCONTACT_DEFAULT;

		// trigger the contact callback for pairs (A,B) where
		// the filtermask of A contains the ID of B and vice versa.
		if ((filterData0.word0 & filterData1.word1) && (filterData1.word0 & filterData0.word1))
			pairFlags |= physx::PxPairFlag::eNOTIFY_TOUCH_FOUND;

		return physx::PxFilterFlag::eDEFAULT;
	}
};


} // NS PX