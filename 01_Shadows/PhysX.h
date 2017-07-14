#pragma once

//PhysX
#include <PxPhysicsAPI.h>

#include "ChunkManager.h"
#include "Controller.h"
#include "GameObjects.h"

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
	physx::PxCooking*				mCooking = NULL;
	physx::PxControllerManager*     mControllerManager = NULL;

	physx::PxCapsuleController*		mController = NULL;
	glm::vec3 forwardVec;
	AnimatedCharacter *				player = nullptr;
public:
	void initPhysics(bool interactive)
	{
		forwardVec = glm::vec3(0,0,1);
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

		gScene->setFlag(physx::PxSceneFlag::eENABLE_ACTIVETRANSFORMS, true);

		mControllerManager = PxCreateControllerManager(*gScene);
	}
	void stepPhysics(float deltaTime, bool interactive, glm::vec3& cowboyPos, Engine::Controller const& controller)
	{
		//const double dTime = 1.0f / 60.0f;
		const double dTime = deltaTime; //TODO Fix timestepping algorithm
		gScene->simulate(dTime);
		gScene->fetchResults(true);

		size_t numberOfTransforms;
		const physx::PxActiveTransform* transforms = gScene->getActiveTransforms(numberOfTransforms);

		if (numberOfTransforms > 0)
		{
			physx::PxActiveTransform transform = transforms[0];
			glm::vec3 pos = Util::PhysXVec3ToglmVec3(transform.actor2World.p);
			glm::quat quat = Util::PhysXQuatToglmQuat(transform.actor2World.q);
			cowboyPos = pos - glm::vec3(0,9,0);
		}
		//Handle controller movement
		cowboyPos = Util::PhysXVec3ToglmVec3(mController->getFootPosition ());
		
		physx::PxVec3 dispCurStep {0,-1,0};
		//dispCurStep += createDisplacementVector (controller); //wasd move
		dispCurStep += createDisplacementVectorResident (controller, dTime);
		dispCurStep *= 0.1;
		
		const physx::PxU32 flags = mController->move(dispCurStep, 0, dTime, physx::PxControllerFilters());
	}
	physx::PxVec3 createDisplacementVector(Engine::Controller const& controller)
	{
		glm::vec3 vec(0);
		vec.x = -(float)controller.isLeft + controller.isRight;
		vec.z = -(float)controller.isForward + controller.isBack;

		return Util::glmVec3ToPhysXVec3(vec);
	}
	physx::PxVec3 createDisplacementVectorResident(Engine::Controller const& controller, float dt)
	{
		modifyForwardVec(controller, dt);
		
		glm::vec3 vec(0);
		vec = forwardVec * (float)controller.isForward - forwardVec * (float)controller.isBack;
		Util::BasisVectors basisVectors = Util::BasisVectors::create (forwardVec);
		vec += basisVectors.right * (float)controller.isRightStrafe - basisVectors.right * (float)controller.isLeftStrafe;

		return Util::glmVec3ToPhysXVec3(vec);
	}
	void modifyForwardVec (Engine::Controller const& controller, float dt)
	{
		static const glm::vec3 BeginForwardVec (0, 0, 1);
		glm::mat4 matrix = glm::rotate (player->yaw, glm::vec3(0,1,0));
		forwardVec = matrix * glm::vec4(BeginForwardVec, 0);
	}

	void cleanupPhysics(bool interactive)
	{
		PX_UNUSED(interactive);
		mCooking->release();
		mControllerManager->release();

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
	void createChunk(Chunk const& chunk)
	{
		const float HalfExtent = chunk.BlockSize * 0.99998;
		physx::PxShape* shape = gPhysics->createShape(physx::PxBoxGeometry(HalfExtent, HalfExtent, HalfExtent), *gMaterial);
		const size_t cubeSize = chunk.GetCubeSize ();
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
						physx::PxTransform localTm(physx::PxVec3(data.pos.x, data.pos.y, data.pos.z));
						//physx::PxRigidDynamic* body = gPhysics->createRigidDynamic(t.transform(localTm));
						physx::PxRigidStatic* body = gPhysics->createRigidStatic(t.transform(localTm));
						body->attachShape(*shape);
						body->setActorFlag(physx::PxActorFlag::eDISABLE_GRAVITY, true);
						//physx::PxRigidBodyExt::updateMassAndInertia(*body, 10.0f);
						gScene->addActor(*body);
					}
				}
			}
		}

		shape->release();
	}
	void createCharacter (glm::vec3 pos,glm::quat rot, AssimpModel * assimpModel, AnimatedCharacter * player)
	{
		this->player = player;
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
		desc.maxJumpHeight = 5.0f;
		desc.reportCallback = NULL; // Meg lehet adni neki osztalyt

		mController = static_cast<physx::PxCapsuleController*>(mControllerManager->createController(desc));
	}
private:
	void createCharacterDynamic (glm::vec3 pos, glm::quat rot, AssimpModel * assimpModel)
	{
		//const std::vector<glm::vec3> vertexes = assimpModel->meshes[0].buffer.GetPositionData();
		//physx::PxConvexMeshDesc convexDesc;
		//convexDesc.points.count = vertexes.size();
		//convexDesc.points.stride = sizeof(glm::vec3); //Vertexek mérete (4*3=12 bájt)
		//convexDesc.points.data = &(vertexes[0]); //Egy float tömb amely a vertexeket tartalmazza
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