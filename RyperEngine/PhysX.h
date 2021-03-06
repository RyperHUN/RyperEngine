#pragma once

//PhysX
#include <PxPhysicsAPI.h>

#include "ChunkManager.h"
#include "Controller.h"
#include "GameObjects.h"

#include "PxController.h"
#include "PxCollisionManager.h"
#include "Events.h"

#define PVD_HOST "127.0.0.1"

class PhysX : public Event::IBlockChanged
{
	physx::PxDefaultAllocator		gAllocator;
	physx::PxDefaultErrorCallback	gErrorCallback;

	physx::PxFoundation*			gFoundation = NULL;
	physx::PxPhysics*				gPhysics = NULL;

	physx::PxDefaultCpuDispatcher*	gDispatcher = NULL;
	physx::PxScene*				    gScene = NULL;

	physx::PxMaterial*				gMaterial = NULL;

	physx::PxPvd*                   gPvd = NULL;
	physx::PxCooking*				mCooking = NULL;
	physx::PxControllerManager*     mControllerManager = NULL;

	physx::PxShape*					mBlockShape = nullptr;

	PX::FPSController				mController;
	PX::CollisionManager			mCollisionManager;
	const float OPTIMAL_FIXED_TIME = 1.0f / 60.0f;
public:
	void initPhysics(bool interactive)
	{
		gFoundation = PxCreateFoundation(PX_FOUNDATION_VERSION, gAllocator, gErrorCallback);

		//ProfileZoneManager???
		gPvd = physx::PxCreatePvd(*gFoundation);
		physx::PxPvdTransport* transport = physx::PxDefaultPvdSocketTransportCreate(PVD_HOST, 5425, 10);
		if(!gPvd->connect(*transport, physx::PxPvdInstrumentationFlag::eALL))
			std::cout << "Physx::PVD - Failed to connect to PhysX Visual Debugger" << std::endl;

		//PxToleranceScale atallithato ha masfajta unitokat akarunk!
		bool recordMemoryAllocations = true;
		physx::PxTolerancesScale scale = physx::PxTolerancesScale();
		gPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *gFoundation, scale , recordMemoryAllocations, gPvd);
		mCooking = PxCreateCooking(PX_PHYSICS_VERSION, *gFoundation, physx::PxCookingParams(scale));
		if (!mCooking) {
			std::cout << "Physx::PVD - Failed to init cooking library" << std::endl;
			assert(false);
		}

		//Fontos hogy ennek a Desc nek meg kell egyeznie a PxTolerancesScale-el
		physx::PxSceneDesc sceneDesc(gPhysics->getTolerancesScale());
		sceneDesc.gravity = physx::PxVec3(0.0f, -9.81f, 0.0f);
		gDispatcher = physx::PxDefaultCpuDispatcherCreate(2);
		sceneDesc.cpuDispatcher = gDispatcher;
		//sceneDesc.filterShader = physx::PxDefaultSimulationFilterShader;
		sceneDesc.filterShader = PX::CollisionManager::SampleFilterShader;
		sceneDesc.simulationEventCallback = (physx::PxSimulationEventCallback*)(&mCollisionManager);
		gScene = gPhysics->createScene(sceneDesc);

		physx::PxPvdSceneClient* pvdClient = gScene->getScenePvdClient();
		if (pvdClient)
		{
			pvdClient->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
			pvdClient->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
			pvdClient->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
		}
		gMaterial = gPhysics->createMaterial(0.5f, 0.5f, 0.6f);

		physx::PxRigidStatic* groundPlane = PxCreatePlane(*gPhysics, physx::PxPlane(0, 1, 0, 0), *gMaterial);
		gScene->addActor(*groundPlane);

		const float HalfExtent = Chunk::wHalfExtent;
		mBlockShape = gPhysics->createShape(physx::PxBoxGeometry(HalfExtent, HalfExtent, HalfExtent), *gMaterial);

		//for (physx::PxU32 i = 0; i<5; i++)
			//createStack(physx::PxTransform(physx::PxVec3(0, 0, 10)), 10, 2.0f);

		//if (!interactive)
		//	createDynamic(PxTransform(PxVec3(0, 40, 100)), PxSphereGeometry(10), PxVec3(0, -50, -100));

		gScene->setFlag(physx::PxSceneFlag::eENABLE_ACTIVETRANSFORMS, true);

		mControllerManager = PxCreateControllerManager(*gScene);
		mCollisionManager.SetScene(gScene);
	}
	float sumTime = 0;
	void stepPhysics(float deltaTime, bool interactive, Engine::Controller & controller)
	{
		if (FixedTimeStep (deltaTime))
		{
			gScene->simulate(OPTIMAL_FIXED_TIME);
			gScene->fetchResults(true);

			//This is only needed if the player is dynamic
			/*size_t numberOfTransforms;
			const physx::PxActiveTransform* transforms = gScene->getActiveTransforms(numberOfTransforms);

			if (numberOfTransforms > 0)
			{
				physx::PxActiveTransform transform = transforms[0];
				glm::vec3 pos = Util::PhysXVec3ToglmVec3(transform.actor2World.p);
				glm::quat quat = Util::PhysXQuatToglmQuat(transform.actor2World.q);
				cowboyPos = pos - glm::vec3(0,9,0);
			}*/
			if (mController.IsValid ())
				mController.Step (OPTIMAL_FIXED_TIME, controller);
		}
	}

	void cleanupPhysics(bool interactive)
	{
		PX_UNUSED(interactive);
		mCooking->release();
		mControllerManager->release();
		mBlockShape->release ();

		gScene->release();
		gDispatcher->release();
		gPhysics->release();
		physx::PxPvdTransport* transport = gPvd->getTransport();
		gPvd->release();
		transport->release();

		gFoundation->release();
	}
	void createStack(const physx::PxTransform& t, physx::PxU32 size, physx::PxReal halfExtent)
	{
		physx::PxShape* shape = gPhysics->createShape(physx::PxBoxGeometry(halfExtent, halfExtent, halfExtent), *gMaterial);
		for (physx::PxU32 i = 0; i<size; i++)
		{
			for (physx::PxU32 j = 0; j<size - i; j++)
			{
				physx::PxTransform localTm(physx::PxVec3(physx::PxReal(j * 2) - physx::PxReal(size - i), physx::PxReal(i * 2 + 1), 0) * halfExtent);
				physx::PxRigidDynamic* body = gPhysics->createRigidDynamic(t.transform(localTm));
				body->attachShape(*shape);
				physx::PxRigidBodyExt::updateMassAndInertia(*body, 10.0f);
				gScene->addActor(*body);
			}
		}
		shape->release();
	}
	//void createChunk(Chunk & chunk)
	//{
	//	const size_t cubeSize = chunk.GetCubeSize ();
	//	for (int k = 0; k < cubeSize; k++)
	//	{
	//		for (int i = 0; i < cubeSize; i++) //row
	//		{
	//			for (int j = 0; j < cubeSize; j++)
	//			{
	//				BlockData& data = chunk.GetBlockData({i,j,k});
	//				AddBLock (data, BlockData::GetWorldPos(chunk.GetWorldPos (), {i,j,k}, Chunk::wHalfExtent * 2.0f));
	//			}
	//		}
	//	}
	//}
	virtual void BlockChangedHandler(BlockData& data, glm::vec3 wBlockPos) override
	{
		if (data.physxPtr != nullptr && !data.isExist)
		{
			physx::PxRigidStatic* body = (physx::PxRigidStatic*)data.physxPtr;
			gScene->removeActor (*body);
			data.physxPtr = nullptr;
		}
		else if (data.physxPtr == nullptr && data.isExist)
		{
			AddBLock (data, wBlockPos);
		}
	}
	void createCharacter (glm::vec3 pos,glm::quat rot, AssimpModel * assimpModel, AnimatedCharacter * player)
	{
		//mController.player = player;
		//createCharacterDynamic(pos,rot, assimpModel);
		physx::PxCapsuleControllerDesc desc;

		//TODO Fix gap between ground
		Geom::Box charBox;
		desc.height = 5.0f; //TODO Little bit smaller capsule
		desc.radius = 3.0f; 
		desc.material = gMaterial;
		desc.position = physx::PxExtendedVec3 (pos.x, pos.y, pos.z);
		desc.slopeLimit = 0.00001f;
		desc.contactOffset = 0.00001f;
		desc.stepOffset = 0.00001f;
		desc.invisibleWallHeight = 0.000000001f;
		desc.maxJumpHeight = 10.0f;
		desc.reportCallback = NULL; // Meg lehet adni neki osztalyt

		mController.mController = static_cast<physx::PxCapsuleController*>(mControllerManager->createController(desc));
	}
	void addPickup (glm::vec3 pos, float radius)
	{
		physx::PxShape* shape = gPhysics->createShape(physx::PxSphereGeometry(radius), *gMaterial, true);
		shape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, false);
		shape->setFlag (physx::PxShapeFlag::eTRIGGER_SHAPE, true);
		physx::PxTransform localTm(Util::glmVec3ToPhysXVec3(pos));

		physx::PxTransform t = physx::PxTransform(physx::PxVec3(0, 0, 0));
		physx::PxRigidStatic* body = gPhysics->createRigidStatic(t.transform(localTm));

		body->attachShape(*shape);
		body->setActorFlag(physx::PxActorFlag::eDISABLE_GRAVITY, true);
		
		gScene->addActor(*body);
		mCollisionManager.SetPickup(body);
	}
	void createFPSCharacter(glm::vec3& pos, glm::vec3& forwardVec)
	{
		mController.playerPos   = &pos;
		mController.forwardVec  = &forwardVec;
		physx::PxCapsuleControllerDesc desc;

		//TODO Fix gap between ground
		Geom::Box charBox;
		desc.height = 5.0f; //TODO Little bit smaller capsule
		desc.radius = 3.0f;
		desc.material = gMaterial;
		desc.position = physx::PxExtendedVec3(pos.x, pos.y, pos.z);
		desc.slopeLimit = 0.00001f;
		desc.contactOffset = 0.00001f;
		desc.stepOffset = 0.00001f;
		desc.invisibleWallHeight = 0.000000001f;
		desc.maxJumpHeight = 10.0f;
		desc.reportCallback = NULL; // Meg lehet adni neki osztalyt

		mController.mController = static_cast<physx::PxCapsuleController*>(mControllerManager->createController(desc));
		mCollisionManager.SetPlayer(mController.mController->getActor());
	}
private:
	void AddBLock (BlockData& data,glm::vec3 wBlockPos)
	{	
		if (data.isExist)
		{
			physx::PxTransform localTm(Util::glmVec3ToPhysXVec3(wBlockPos)); ///TODO need world pos of chunks
																			   //physx::PxRigidDynamic* body = gPhysics->createRigidDynamic(t.transform(localTm));
			physx::PxTransform t = physx::PxTransform(physx::PxVec3(0, 0, 0));
			physx::PxRigidStatic* body = gPhysics->createRigidStatic(t.transform(localTm));
			data.physxPtr = body; //This is why we use non const getter
			body->attachShape(*mBlockShape);
			body->setActorFlag(physx::PxActorFlag::eDISABLE_GRAVITY, true);
			//physx::PxRigidBodyExt::updateMassAndInertia(*body, 10.0f);
			gScene->addActor(*body);
		}
	}
	bool FixedTimeStep (float deltaTime)
	{
		sumTime += deltaTime;
		if (OPTIMAL_FIXED_TIME < sumTime )
		{
			sumTime = sumTime - OPTIMAL_FIXED_TIME;
			return true;
		}
		return false;
	}
	void createCharacterDynamic (glm::vec3 pos, glm::quat rot, AssimpModel * assimpModel)
	{
		//const std::vector<glm::vec3> vertexes = assimpModel->meshes[0].buffer.GetPositionData();
		//physx::PxConvexMeshDesc convexDesc;
		//convexDesc.points.count = vertexes.size();
		//convexDesc.points.stride = sizeof(glm::vec3); //Vertexek m�rete (4*3=12 b�jt)
		//convexDesc.points.data = &(vertexes[0]); //Egy float t�mb amely a vertexeket tartalmazza
		//convexDesc.flags = physx::PxConvexFlag::eCOMPUTE_CONVEX;

		//physx::PxDefaultMemoryOutputStream buf;
		//if (mCooking->cookConvexMesh(convexDesc, buf))
		//{
		//	physx::PxDefaultMemoryInputData input(buf.getData(), buf.getSize());
		//	physx::PxConvexMesh* convexMesh = gPhysics->createConvexMesh(input);

		//	physx::PxTransform t = physx::PxTransform(physx::PxVec3(0, 0, 0));
		//	physx::PxTransform localTm(Util::glmVec3ToPhysXVec3(pos));
		//	physx::PxQuat quat = Util::glmQuatToPhysXQuat(rot);
		//	physx::PxTransform localRotate (quat);

		//	physx::PxRigidDynamic* body = gPhysics->createRigidDynamic(t.transform(localTm).transform(localRotate));
		//	
		//	body->setActorFlag(physx::PxActorFlag::eDISABLE_GRAVITY, false);
		//	body->setAngularVelocity (physx::PxVec3(0,0,0));
		//	physx::PxRigidBodyExt::updateMassAndInertia(*body, 10.0f);
		//	body->setMass(100);
		//	body->setCMassLocalPose(physx::PxTransform(physx::PxVec3(0, 0, 0)));
		//	body->setMassSpaceInertiaTensor(physx::PxVec3(0, 0, 0));

		//	physx::PxShape* shape = body->createShape (physx::PxConvexMeshGeometry(convexMesh), *gMaterial);

		//	gScene->addActor (*body);
		//}
	}
};