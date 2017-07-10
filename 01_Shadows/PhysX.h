#pragma once

//PhysX
#include <PxPhysicsAPI.h>

#include "ChunkManager.h"


#define PVD_HOST "127.0.0.1"

class PhysX 
{
	physx::PxDefaultAllocator		gAllocator;
	physx::PxDefaultErrorCallback	gErrorCallback;

	physx::PxFoundation*			gFoundation = NULL;
	physx::PxPhysics*				gPhysics = NULL;

	physx::PxDefaultCpuDispatcher*	gDispatcher = NULL;
	physx::PxScene*				    gScene = NULL;

	physx::PxMaterial*				gMaterial = NULL;

	physx::PxPvd*                   gPvd = NULL;
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
		gPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *gFoundation, physx::PxTolerancesScale(), recordMemoryAllocations, gPvd);

		//Fontos hogy ennek a Desc nek meg kell egyeznie a PxTolerancesScale-el
		physx::PxSceneDesc sceneDesc(gPhysics->getTolerancesScale());
		sceneDesc.gravity = physx::PxVec3(0.0f, -9.81f, 0.0f);
		gDispatcher = physx::PxDefaultCpuDispatcherCreate(2);
		sceneDesc.cpuDispatcher = gDispatcher;
		sceneDesc.filterShader = physx::PxDefaultSimulationFilterShader;
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

		//for (physx::PxU32 i = 0; i<5; i++)
			//createStack(physx::PxTransform(physx::PxVec3(0, 0, 10)), 10, 2.0f);

		//if (!interactive)
		//	createDynamic(PxTransform(PxVec3(0, 40, 100)), PxSphereGeometry(10), PxVec3(0, -50, -100));
	}
	void stepPhysics(bool interactive)
	{
		gScene->simulate(1.0f / 60.0f);
		gScene->fetchResults(true);
	}

	void cleanupPhysics(bool interactive)
	{
		PX_UNUSED(interactive);
		gScene->release();
		gDispatcher->release();
		gPhysics->release();
		physx::PxPvdTransport* transport = gPvd->getTransport();
		gPvd->release();
		transport->release();

		gFoundation->release();

		printf("SnippetHelloWorld done.\n");
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
	void createChunk(Chunk const& chunk)
	{
		const float HalfExtent = chunk.BlockSize * 0.99998;
		physx::PxShape* shape = gPhysics->createShape(physx::PxBoxGeometry(HalfExtent, HalfExtent, HalfExtent), *gMaterial);
		const size_t cubeSize = chunk.GetCubeSize ();
		int drawedShaped = 0;
		for (int k = 0; k < cubeSize; k++)
		{
			for (int i = 0; i < cubeSize; i++) //row
			{
				for (int j = 0; j < cubeSize; j++)
				{
					physx::PxTransform t = physx::PxTransform(physx::PxVec3(0, 0, 0));
					ChunkData const& data = chunk.chunkInfo[i][j][k];
					if(data.isExist)
					{
						physx::PxTransform localTm(physx::PxVec3(data.pos.x, data.pos.z, data.pos.y));
						//physx::PxRigidDynamic* body = gPhysics->createRigidDynamic(t.transform(localTm));
						physx::PxRigidStatic* body = gPhysics->createRigidStatic(t.transform(localTm));
						body->attachShape(*shape);
						body->setActorFlag(physx::PxActorFlag::eDISABLE_GRAVITY, true);
						//physx::PxRigidBodyExt::updateMassAndInertia(*body, 10.0f);
						gScene->addActor(*body);
						drawedShaped++;
					}
				}
			}
		}

		shape->release();
	}
};