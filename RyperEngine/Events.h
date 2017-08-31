#pragma once

//Predeclarations
struct BlockData;

namespace Event
{

struct IEventHandler {};


struct IBlockChanged : IEventHandler
{
	virtual void BlockChangedHandler (BlockData&, glm::vec3) = 0;
};

}