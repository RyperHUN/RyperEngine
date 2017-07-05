#pragma once

#include <GL\glew.h>
#include <SDL.h>

#include "glmIncluder.h"
#include "UtilityFuncs.h"
#include "Drawer.h"
#include <string>
#include <functional>
#include "TextRenderer.h"

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
	virtual void Draw(glm::ivec2 screenSize, QuadTexturer &texturer, TextRenderer &textRenderer) {}

	static glm::mat4 GetModelTransform(glm::ivec2 const& pos, 
		glm::ivec2 const& size, glm::ivec2 const& screenSize)
	{
		//Full screen quad to local size
		glm::vec3 scale = glm::vec3(glm::vec2(size) / glm::vec2(screenSize), 1);
		//Origo to the top left corner
		glm::vec3 translateOrigo = glm::vec3(scale.x, -scale.y, 0);
		glm::vec3 translatePos = glm::vec3(Util::pixelToNdc(pos, screenSize), 0);

		glm::mat4 model = glm::translate(translatePos)*
			glm::translate(translateOrigo)*
			glm::scale(scale);

		return model;
	}
};

struct ButtonA : public Widget
{
	std::string text;
	std::function<void()> callback;
	ButtonA (glm::ivec2 pos, glm::ivec2 size, std::string text, std::function<void()> fv = [](){})
		:Widget(pos, size), text(text), callback(fv)
	{}

	bool isClicked;
	virtual void MouseDown(SDL_MouseButtonEvent& mouse) override
	{
		if (mouse.button == SDL_BUTTON_LEFT && isInsideMouse (mouse.x, mouse.y))
		{
			callback();
			isClicked = !isClicked;
		}
	}
	virtual void Draw (glm::ivec2 screenSize, QuadTexturer &texturer, TextRenderer &textRenderer) override
	{
		glDisable(GL_DEPTH_TEST);
		glm::mat4 model = Widget::GetModelTransform(pos, size, screenSize);

		//TODO Render texture
		if(isClicked)
			texturer.Draw(glm::vec4(0,1,0,1), model);
		else
			texturer.Draw(glm::vec4(1, 0, 0, 1), model);

		TextData data = textRenderer.RenderStrToTexture (text);
		//data.size     = size;
		if(data.texCoord != -1)
			textRenderer.RenderStr (std::move(data), model);
		glEnable(GL_DEPTH_TEST);
	}
};

struct Checkbox : public Widget
{
	ButtonA button;
	bool * value;
	std::string text;
	Checkbox(glm::ivec2 pos, glm::ivec2 size, std::string text, bool * value)
		:Widget(pos, size),
		button(pos, glm::ivec2(size.y,size.y), ""),
		text(text),
		value(value)
	{
		button.callback  = [value, this](){
			*value = !(*value);
			if (*value)
				button.text = "X"; //Checkbox like text
			else
				button.text = "";
		};
		button.isClicked = *value;
	}
	virtual void MouseDown(SDL_MouseButtonEvent& mouse) override
	{
		button.MouseDown (mouse);
	}
	virtual void Draw(glm::ivec2 screenSize, QuadTexturer &texturer, TextRenderer &textRenderer) override
	{
		button.Draw (screenSize, texturer, textRenderer);

		//TODO Render text
		TextData data = textRenderer.RenderStrToTexture (text);
		size.x = size.y + data.size.x;

		glm::ivec2 textPos = glm::ivec2(pos.x + size.y, pos.y);
		glm::ivec2 textSize = glm::ivec2(size.x - size.y, size.y);
		glm::mat4 model = Widget::GetModelTransform(textPos, textSize, screenSize);

		glEnable(GL_BLEND);
		texturer.Draw (data.texCoord,false, model);
		glDisable(GL_BLEND);

		glDeleteTextures (1,&data.texCoord);
	}
};

