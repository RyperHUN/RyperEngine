#pragma once

#include <SDL.h>

namespace Engine {

struct Controller 
{
	bool isLeft    = false;
	bool isRight   = false;
	bool isForward = false;
	bool isBack    = false;
	bool isRightStrafe = false;
	bool isLeftStrafe  = false;
	void KeyboardDown(SDL_KeyboardEvent& key)
	{
		HandleButton (key, true);
	}
	void KeyboardUp(SDL_KeyboardEvent& key)
	{
		HandleButton(key, false);
	}
	void HandleButton(SDL_KeyboardEvent const& key, bool val)
	{
		switch (key.keysym.sym)
		{
		case SDLK_w:
			isForward = val;
			break;
		case SDLK_s:
			isBack = val;
			break;
		case SDLK_a:
			isLeft = val;
			break;
		case SDLK_d:
			isRight = val;
			break;
		case SDLK_e:
			isRightStrafe = val;
			break;
		case SDLK_q:
			isLeftStrafe = val;
			break;
		}
	}
};

} //NS Engine