#pragma once

#include <GL\glew.h>
#include <SDL.h>

#include "glmIncluder.h"
#include "UtilityFuncs.h"
#include "Drawer.h"
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
	virtual void Draw(glm::ivec2 screenSize, QuadTexturer &texturer) {}
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
	virtual void Draw (glm::ivec2 screenSize, QuadTexturer &texturer) override
	{
		//Full screen quad to local size
		glm::vec3 scale = glm::vec3(glm::vec2(size) / glm::vec2(screenSize), 1);
		//Origo to the top left corner
		glm::vec3 translateOrigo = glm::vec3(scale.x, scale.y, 0);
		glm::vec3 translatePos   = glm::vec3(Util::pixelToNdc (pos, screenSize), 0);

		glm::mat4 model = glm::translate(translatePos)*
						  glm::translate(translateOrigo)* 
			              glm::scale(scale);

		//TODO Render texture
		texturer.Draw (glm::vec4(1,0,0,1),model);
	}
};

