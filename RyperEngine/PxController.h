#pragma once

#include <PxPhysicsAPI.h>
#include "glmIncluder.h"
#include "GameObjects.h"
#include "PxJump.h"

namespace PX
{

struct FPSController
{
	glm::vec3 * playerPos = nullptr;
	glm::vec3 const * forwardVec = nullptr;
	physx::PxCapsuleController*		mController = nullptr;
	float							mGravity = -9.81f;
	float							mJumpForce = 60.0f;
	PX::Jump						mJump;


	bool IsValid()
	{
		return mController != nullptr && playerPos != nullptr && forwardVec != nullptr;
	}

	void Step(float dTime, Engine::Controller & controller)
	{
		//Handle controller movement
		*playerPos = Util::PhysXVec3ToglmVec3(mController->getPosition()) + glm::vec3(0,2,0);

		physx::PxVec3 dispCurStep{ 0,0,0 };
		const PxF32 heightDelta = mJump.getHeight(dTime);
		float dy;
		if (heightDelta != 0.0f)
			dy = heightDelta;
		else
			dy = mGravity * dTime;

		dispCurStep.y = dy;

		//dispCurStep += createDisplacementVector (controller); //wasd move
		dispCurStep += createDisplacementVectorResident(controller, dTime) * 0.4;


		physx::PxControllerState cctState; //Get controller is on ground or not!
		mController->getState(cctState);
		//if ((cctState.collisionFlags & physx::PxControllerCollisionFlag::eCOLLISION_DOWN) == 0) //If not on ground
		//	dispCurStep.y += -1; //Add gravity

		//dispCurStep *= 0.1;
		const physx::PxU32 flags = mController->move(dispCurStep, 0, dTime, physx::PxControllerFilters());

		if ((flags & physx::PxControllerCollisionFlag::eCOLLISION_DOWN) != 0) //If on ground
			mJump.stopJump();

		if (controller.isJump)
		{
			controller.isJump = false;
			mJump.startJump(mJumpForce);
		}
	}
private:
	physx::PxVec3 createDisplacementVector(Engine::Controller const& controller)
	{
		glm::vec3 vec(0);
		vec.x = -(float)controller.isLeft + controller.isRight;
		vec.z = -(float)controller.isForward + controller.isBack;

		return Util::glmVec3ToPhysXVec3(vec);
	}
	physx::PxVec3 createDisplacementVectorResident(Engine::Controller const& controller, float dt)
	{
		glm::vec3 forwardXZ = glm::vec3(forwardVec->x, 0, forwardVec->z);

		glm::vec3 vec(0);
		vec = forwardXZ * (float)controller.isForward - forwardXZ * (float)controller.isBack;
		Util::BasisVectors basisVectors = Util::BasisVectors::create(forwardXZ);
		vec += basisVectors.right * (float)controller.isRightStrafe - basisVectors.right * (float)controller.isLeftStrafe;

		return Util::glmVec3ToPhysXVec3(vec);
	}
};

struct Controller
{
	glm::vec3 forwardVec;
	AnimatedCharacter *				player      = nullptr;
	physx::PxCapsuleController*		mController = nullptr;
	float							mGravity    =-9.81f;
	float							mJumpForce  = 60.0f;
	PX::Jump						mJump;

	bool IsValid ()
	{
		return player != nullptr && mController != nullptr;
	}

	void Step (float dTime, glm::vec3 &cowboyPos, Engine::Controller & controller)
	{
		//Handle controller movement
		cowboyPos = Util::PhysXVec3ToglmVec3(mController->getFootPosition());

		physx::PxVec3 dispCurStep{ 0,0,0 };
		const PxF32 heightDelta = mJump.getHeight(dTime);
		float dy;
		if (heightDelta != 0.0f)
			dy = heightDelta;
		else
			dy = mGravity * dTime;

		dispCurStep.y = dy;

		//dispCurStep += createDisplacementVector (controller); //wasd move
		dispCurStep += createDisplacementVectorResident(controller, dTime) * 0.2;


		physx::PxControllerState cctState; //Get controller is on ground or not!
		mController->getState(cctState);
		//if ((cctState.collisionFlags & physx::PxControllerCollisionFlag::eCOLLISION_DOWN) == 0) //If not on ground
		//	dispCurStep.y += -1; //Add gravity

		//dispCurStep *= 0.1;
		const physx::PxU32 flags = mController->move(dispCurStep, 0, dTime, physx::PxControllerFilters());

		if ((flags & physx::PxControllerCollisionFlag::eCOLLISION_DOWN) != 0) //If on ground
			mJump.stopJump();

		if(controller.isJump)
		{
			controller.isJump = false;
			mJump.startJump(mJumpForce);
		}
	}
private:
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
		Util::BasisVectors basisVectors = Util::BasisVectors::create(forwardVec);
		vec += basisVectors.right * (float)controller.isRightStrafe - basisVectors.right * (float)controller.isLeftStrafe;

		return Util::glmVec3ToPhysXVec3(vec);
	}
	void modifyForwardVec(Engine::Controller const& controller, float dt)
	{
		static const glm::vec3 BeginForwardVec(0, 0, 1);
		glm::mat4 matrix = glm::rotate(player->yaw, glm::vec3(0, 1, 0));
		forwardVec = matrix * glm::vec4(BeginForwardVec, 0);
	}
};

}// NS PX