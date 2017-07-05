#pragma once

#include <GL\glew.h>
#include <SDL.h>

#include "glmIncluder.h"
#include "UtilityFuncs.h"
#include <string>
#include <functional>

struct Widget {
	glm::ivec2 pos;
	glm::ivec2 size;

	Widget (glm::ivec2 pos, glm::ivec2 size)
		: pos(pos), size(size)
	{}

	bool isSelected = false;
	bool isVisible  = true;

	virtual bool isInsideMouse(int x, int y) 
	{
		return pos.x <= x && pos.x + size.x >= x &&
			   pos.y <= y && pos.y + size.y >= y;
	}
	virtual void MouseMove(SDL_MouseMotionEvent&) {};
	virtual void MouseDown(SDL_MouseButtonEvent&) {};
	virtual void MouseUp(SDL_MouseButtonEvent&) {};
	virtual void MouseWheel(SDL_MouseWheelEvent&) {};
	virtual void Draw(glm::ivec2 screenSize) {}
};

struct ButtonA : public Widget
{
	std::string text;
	std::function<void()> callback;
	ButtonA (glm::ivec2 pos, glm::ivec2 size, std::string text, std::function<void()> fv = [](){})
		:Widget(pos, size), text(text), callback(fv)
	{}
	virtual void MouseDown(SDL_MouseButtonEvent& mouse) override
	{
		if (mouse.button == SDL_BUTTON_LEFT && isInsideMouse (mouse.x, mouse.y))
		{
			callback();
		}
	}
	virtual void Draw (glm::ivec2 screenSize) override
	{
		//Full screen quad to local size
		glm::vec2 scale = glm::vec2(size) / glm::vec2(screenSize);
		//Origo to the top left corner
		glm::vec3 translate = glm::vec3(scale.x, scale.y, 0);

	}
};

