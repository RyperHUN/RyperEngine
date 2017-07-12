#pragma once

#include <SDL.h>

namespace Engine {

struct Controller 
{
	bool isLeft    = false;
	bool isRight   = false;
	bool isForward = false;
	bool isBack    = false;
	void KeyboardDown(SDL_KeyboardEvent& key)
	{
		switch (key.keysym.sym)
		{
		case SDLK_w:
			isForward = true;
			break;
		case SDLK_s:
			isBack = true;
			break;
		case SDLK_a:
			isLeft = true;
			break;
		case SDLK_d:
			isRight = true;
			break;
		}
	}
	void KeyboardUp(SDL_KeyboardEvent& key)
	{
		switch (key.keysym.sym)
		{
		case SDLK_w:
			isForward = false;
			break;
		case SDLK_s:
			isBack = false;
			break;
		case SDLK_a:
			isLeft = false;
			break;
		case SDLK_d:
			isRight = false;
			break;
		}
	}

};

} //NS Engine