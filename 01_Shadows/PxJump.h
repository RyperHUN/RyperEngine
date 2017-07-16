#pragma once

#include <PxPhysicsAPI.h>

namespace PX
{
using namespace physx; //TODO Check if it leaks anything?!

static PxF32 gJumpGravity = -50.0f;

class Jump
{
	PxF32		mV0;
	PxF32		mJumpTime;
	bool		mJump;
public:
	Jump()
		:mV0(0.0f),mJumpTime(0.0f),	mJump(false)
	{}

	void startJump(PxF32 v0)
	{
		if (mJump)	return;
		mJumpTime = 0.0f;
		mV0 = v0;
		mJump = true;
	}
	void stopJump()
	{
		if (!mJump)	return;
		mJump = false;
	}
	PxF32 getHeight(PxF32 elapsedTime)
	{
		if (!mJump)	return 0.0f;
		mJumpTime += elapsedTime;
		const PxF32 h = gJumpGravity*mJumpTime*mJumpTime + mV0*mJumpTime;
		return h*elapsedTime;
	}
};

} //NS PX